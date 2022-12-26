
#pragma vertex

#pragma using SceneData // Get the scene data (viewMatrix, projectionMatrix)
#pragma using ModelData // Get the model data (modelMatrix)

layout(location = POSITION) in vec3 aPos;
layout(location = NORMAL) in vec3 aNormal;

out vec3 normal;
out vec3 worldPos;

void main()
{ 
	normal = mat3(modelToWorld) * aNormal;
	worldPos = vec3(modelToWorld * vec4(aPos, 1.0));
	gl_Position = viewToScreen * worldToView * modelToWorld * vec4(aPos, 1.0);
}


#pragma fragment

#pragma using Lighting
#pragma using SceneData
#pragma using PBR

in vec3 normal;
in vec3 worldPos;
out vec4 FragColor; 

const float PI = 3.14159265359;

[Slider(0, 1)]uniform vec3  albedo;
[Slider(0, 1)]uniform float metallic;
[Slider(0, 1)]uniform float roughness;
[Slider(0, 1)]uniform float ao;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{ 
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - worldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04); // Min value for non-metal surfaces
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    // TODO : optimize this :
    
    //for (int i = 0; i < 1; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPos - worldPos); // lightPos[i]
        vec3 H = normalize(V + L);
        float distance = length(lightPos - worldPos); // lightPos[i]
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColor * attenuation; // lightColor[i]

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(PBRIrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    // Specular
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(PBRPrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(PBRBrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
	