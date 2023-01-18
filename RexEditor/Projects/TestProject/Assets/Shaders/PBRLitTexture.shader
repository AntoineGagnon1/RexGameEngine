#pragma vertex

#pragma using SceneData // Get the scene data (viewMatrix, projectionMatrix)
#pragma using ModelData // Get the model data (modelMatrix)

layout(location = POSITION) in vec3 aPos;
layout(location = NORMAL) in vec3 aNormal;
layout(location = TEXCOORDS) in vec2 aUV;

out vec3 normal;
out vec3 worldPos;
out vec2 uv;

void main()
{ 
	uv = aUV;
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
in vec2 uv;
out vec4 FragColor; 

uniform sampler2D albedo;
[Slider(0, 1)]uniform float metallic;
[Slider(0, 1)]uniform float roughness;
[Slider(0, 1)]uniform float ao;

void main()
{
    FragColor = vec4(GetPBRColorLit(texture(albedo, uv).rgb, metallic, roughness, ao, worldPos, cameraPos, normal), 1.0);
}
	