#include "REPch.h"
#include "PBR.h"

#include "Texture.h"
#include "FrameBuffer.h"
#include "RenderBuffer.h"
#include "Shader.h"
#include "Shapes.h"
#include "TextureManager.h"
#include "scene/Scene.h"
#include "scene/Components.h"

#include "window/Window.h"

namespace RexEngine
{
	namespace Internal
	{
		// Used by the projection shader, irradiance shader and the prefilter shader
		const std::string vertexShaderString = R"(
#pragma version 420 core
#pragma vertex
		
layout(location = POSITION) in vec3 aPos;
out vec3 WorldPos;
		
uniform mat4 transformMatrix;
		
void main()
{
	WorldPos = aPos;
	gl_Position = transformMatrix * vec4(WorldPos, 1.0);
}
)";

		const std::string projectionShaderString = R"(
#pragma fragment
out vec4 FragColor;
in vec3 WorldPos;
		
uniform sampler2D equirectangularMap;
		
const vec2 invAtan = vec2(0.1591, 0.3183);
		
void main() // From https://learnopengl.com/PBR/IBL/Diffuse-irradiance
{
	vec3 pos = normalize(WorldPos);
	vec2 uv = vec2(atan(pos.z, pos.x), asin(pos.y));
	uv *= invAtan;
	uv += 0.5;
		
	vec3 color = texture(equirectangularMap, uv).rgb;
	FragColor = vec4(color, 1.0);
}
)";

		const std::string irradianceShaderString = R"(
#pragma fragment
out vec4 FragColor;
in vec3 WorldPos;
		
uniform samplerCube environmentMap;
uniform float sampleDelta;
		
const float PI = 3.14159265359;
		
void main()
{		
	// The world vector acts as the normal of a tangent surface
	// from the origin, aligned to WorldPos. Given this normal, calculate all
	// incoming radiance of the environment. The result of this radiance
	// is the radiance of light coming from -Normal direction, which is what
	// we use in the PBR shader to sample irradiance.
	vec3 N = normalize(WorldPos);
		
	vec3 irradiance = vec3(0.0);   
		    
	// tangent space calculation from origin point
	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up         = normalize(cross(N, right));
		       
	float nrSamples = 0.0;
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
		    // spherical to cartesian (in tangent space)
		    vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
		    // tangent space to world
		    vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 
		
		    irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
		    nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));
		    
	FragColor = vec4(irradiance, 1.0);
}
)";

		// From https://learnopengl.com/code_viewer_gh.php?code=src/6.pbr/2.2.1.ibl_specular/2.2.1.prefilter.fs
		const std::string prefilterShaderString = R"(
#pragma fragment
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float RadicalInverse_VdC(uint bits) 
{ // http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

void main()
{		
    vec3 N = normalize(WorldPos);
    
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
})";

const std::string lutShader = R"(
#pragma version 420 core
#pragma vertex
layout (location = POSITION) in vec3 aPos;
layout (location = TEXCOORDS) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
	gl_Position = vec4(aPos, 1.0);
}

#pragma fragment
out vec4 FragColor;
in vec2 TexCoords;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) 
{ // http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // note that we use a different k for IBL
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec2 IntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0; 

    vec3 N = vec3(0.0, 0.0, 1.0);
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

void main() 
{
    vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
    FragColor = vec4(integratedBRDF, 0, 1);
})";

		const Matrix4 ViewMatrices[] =
		{ // The 6 directions for the 6 faces of the cube
			Matrix4::MakeLookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f,  0.0f,  0.0f), Vector3(0.0f, -1.0f,  0.0f)),
			Matrix4::MakeLookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f,  0.0f), Vector3(0.0f, -1.0f,  0.0f)),
			Matrix4::MakeLookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f,  1.0f,  0.0f), Vector3(0.0f,  0.0f, -1.0f)),
			Matrix4::MakeLookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f,  0.0f), Vector3(0.0f,  0.0f,  1.0f)),
			Matrix4::MakeLookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f,  0.0f, -1.0f), Vector3(0.0f, -1.0f,  0.0f)),
			Matrix4::MakeLookAt(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f,  0.0f,  1.0f), Vector3(0.0f, -1.0f,  0.0f))
		};
	}

	std::shared_ptr<Cubemap> PBR::CreateIrradianceMap(const Cubemap& cubemapFrom, int size, float sampleDelta)
	{
		auto cubemap = std::make_shared<Cubemap>();
		auto cubemapId = cubemap->GetId();

		// Init the cubemap with empty textures
		for (int i = 0; i < 6; i++)
		{
			RenderApi::SetCubemapFace(cubemapId, (RenderApi::CubemapFace)i, RenderApi::PixelFormat::RGB16F, { size, size},
				NULL, RenderApi::PixelFormat::RGB, RenderApi::PixelType::Float);
		}

		Matrix4 projectionMatrix = Matrix4::MakePerspective(90.0f, 1.0f, 0.1f, 10.0f);

		// Create the frame buffer
		RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
		FrameBuffer frameBuffer;
		RenderBuffer renderBuffer(RenderApi::PixelType::Depth, { size , size});
		frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

		// Create the irradiance shader
		static NoDestroy<Shader> irradianceShader(Internal::vertexShaderString + Internal::irradianceShaderString, RenderApi::CullingMode::Back);

		irradianceShader->SetUniformFloat("sampleDelta", sampleDelta);
		irradianceShader->SetUniformInt("environmentMap", 0);
		RenderApi::SetActiveTexture(0);
		cubemapFrom.Bind();

		Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end
		RenderApi::SetViewportSize({ size, size });

		frameBuffer.Bind();
		RenderApi::SetDepthFunction(RenderApi::DepthFunction::LessEqual);
		auto cubeMesh = Shapes::GetCubeMesh();
		cubeMesh->Bind(); // Bind the cube
		for (unsigned int i = 0; i < 6; ++i)
		{
			irradianceShader->SetUniformMatrix4("transformMatrix", projectionMatrix * Internal::ViewMatrices[i]);

			frameBuffer.BindCubemapFace(*cubemap, (RenderApi::CubemapFace)i, RenderApi::FrameBufferTextureType::Color);

			RenderApi::ClearColorBit();
			RenderApi::ClearDepthBit();

			RenderApi::DrawElements(cubeMesh->GetIndexCount()); // Render to the cubemap
		}
		
		RenderApi::BindFrameBuffer(oldFrameBuffer);
		RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size

		return cubemap;
	}

	std::shared_ptr<Cubemap> PBR::CreatePreFilterMap(const Cubemap& cubemapFrom, int size)
	{
		auto cubemap = std::make_shared<Cubemap>();
		auto cubemapId = cubemap->GetId();

		// Init the cubemap with empty textures
		for (int i = 0; i < 6; i++)
		{
			RenderApi::SetCubemapFace(cubemapId, (RenderApi::CubemapFace)i, RenderApi::PixelFormat::RGB16F, { size, size },
				NULL, RenderApi::PixelFormat::RGB, RenderApi::PixelType::Float);
		}

		cubemap->SetOption(RenderApi::TextureOption::MinFilter, RenderApi::TextureOptionValue::LinearMipmap);
		cubemap->GenerateMipmaps();

		Matrix4 projectionMatrix = Matrix4::MakePerspective(90.0f, 1.0f, 0.1f, 10.0f);

		// Create the frame buffer
		RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
		FrameBuffer frameBuffer;
		RenderBuffer renderBuffer(RenderApi::PixelType::Depth, { size, size });
		frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

		// Create the irradiance shader
		static NoDestroy<Shader> prefilterShader(Internal::vertexShaderString + Internal::prefilterShaderString, RenderApi::CullingMode::Back);

		prefilterShader->SetUniformInt("environmentMap", 0);
		RenderApi::SetActiveTexture(0);
		cubemapFrom.Bind();

		Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end

		frameBuffer.Bind();
		RenderApi::SetDepthFunction(RenderApi::DepthFunction::LessEqual);
		auto cubeMesh = Shapes::GetCubeMesh();
		cubeMesh->Bind(); // Bind the cube

		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; mip++)
		{
			// Resize framebuffer according to mip level
			unsigned int mipSize = (unsigned int)(size * std::pow(0.5, mip));

			//renderBuffer.Bind();
			//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			RenderApi::SetViewportSize(Vector2Int(mipSize, mipSize));

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			prefilterShader->SetUniformFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; i++)
			{
				prefilterShader->SetUniformMatrix4("transformMatrix", projectionMatrix * Internal::ViewMatrices[i]);

				frameBuffer.BindCubemapFace(*cubemap, (RenderApi::CubemapFace)i, RenderApi::FrameBufferTextureType::Color, mip);

				RenderApi::ClearColorBit();
				RenderApi::ClearDepthBit();

				RenderApi::DrawElements(cubeMesh->GetIndexCount()); // Render to the cubemap
			}
		}

		RenderApi::BindFrameBuffer(oldFrameBuffer);
		RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size

		return cubemap;
	}

	std::shared_ptr<Texture> PBR::CreateBRDFLut(Vector2Int size)
	{
		auto texture = std::make_shared<Texture>(RenderApi::PixelFormat::RGB16F, size);
		texture->SetOption(RenderApi::TextureOption::WrapS, RenderApi::TextureOptionValue::ClampToEdge);
		texture->SetOption(RenderApi::TextureOption::WrapT, RenderApi::TextureOptionValue::ClampToEdge);

		// Configure capture framebuffer object and render screen-space quad with BRDF shader.
		RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
		Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end
		
		FrameBuffer frameBuffer;
		RenderBuffer renderBuffer(RenderApi::PixelType::Depth, size);
		frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);
		frameBuffer.BindTexture(*texture, RenderApi::FrameBufferTextureType::Color);

		static NoDestroy<Shader> shader(Internal::lutShader, RenderApi::CullingMode::Front);

		auto quad = Shapes::GetQuadMesh();
		quad->Bind();

		frameBuffer.Bind();
		RenderApi::SetViewportSize(size);
		RenderApi::SetDepthFunction(RenderApi::DepthFunction::LessEqual);

		shader->Bind();
		RenderApi::ClearColorBit();
		RenderApi::ClearDepthBit();
		RenderApi::DrawElements(quad->GetIndexCount());

		RenderApi::BindFrameBuffer(oldFrameBuffer);
		RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size

		return texture;
	}

	void PBR::Init()
	{
		// Reserve the slots
		s_irradianceMapSlot = TextureManager::ReserveSlot();
		s_prefilterMapSlot = TextureManager::ReserveSlot();
		s_brdfLUTSlot = TextureManager::ReserveSlot();

		// Generate and bind the BRDFLut now
		s_brdfLUT = PBR::CreateBRDFLut(Vector2Int(BrdfLutSize, BrdfLutSize));
		RenderApi::SetActiveTexture(s_brdfLUTSlot);
		s_brdfLUT->Bind();
		RenderApi::SetActiveTexture(0);

		// Set the shader data
		Shader::RegisterParserUsing("PBR", std::format(R"(
[Hide]layout(binding = {}) uniform samplerCube PBRIrradianceMap;
[Hide]layout(binding = {}) uniform samplerCube PBRPrefilterMap;
[Hide]layout(binding = {}) uniform sampler2D PBRBrdfLUT;
)", s_irradianceMapSlot, s_prefilterMapSlot, s_brdfLUTSlot) 
+ R"(
const float PI_PBR = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI_PBR * denom * denom;

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

// Returns the radiance, for Point lights and directional lights
float CalculateLight(LightData light, vec3 worldPos, vec3 N, vec3 V, vec3 F0, float metallic, float roughness, vec3 albedo)
{
    vec3 L, H, radiance;
    if (light.Pos.w == 1.0f)
    { // Point light
        L = normalize(light.Pos.xyz - worldPos);
        H = normalize(V + L);
        float distance = length(light.Pos.xyz - worldPos);
        float attenuation = 1.0 / (distance * distance);
        radiance = light.Color * attenuation;
    }
    else
    { // Directional light
        L = normalize(-light.Pos.xyz); // .Pos is the direction
        H = normalize(V + L);
        radiance = light.Color;
    }

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

    // Outgoing radiance
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI_PBR + specular) * radiance * NdotL;
}

// Returns the radiance, for Spot lights
float CalculateSpotLight(SpotLightData light, vec3 worldPos, vec3 N, vec3 V, vec3 F0, float metallic, float roughness, vec3 albedo)
{
    vec3 L = normalize(light.Pos.xyz - worldPos);
    vec3 H = normalize(V + L);

    float theta = dot(L, normalize(-light.Dir.xyz));
    float epsilon = (light.CutOff - light.OuterCutOff);
    float intensity = clamp((theta - light.OuterCutOff) / epsilon, 0.0, 1.0);
    float distance = length(light.Pos.xyz - worldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.Color.xyz * intensity * attenuation;

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

    // Outgoing radiance
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI_PBR + specular) * radiance * NdotL;
}

// Needs using Lighting
vec3 GetPBRColorLit(vec3 albedo, float metallic, float roughness, float ao, vec3 worldPos, vec3 cameraPos, vec3 normal)
{
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - worldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04); // Min value for non-metal surfaces
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    for (int i = 0; i < LightCount; i++)
        Lo += CalculateLight(Lights[i], worldPos, N, V, F0, metallic, roughness, albedo);

    for (int i = 0; i < SpotLightCount; i++)
        Lo += CalculateSpotLight(SpotLights[i], worldPos, N, V, F0, metallic, roughness, albedo);

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
    return color;
}
)");

	}

	void PBR::OnClose()
	{
		s_skyboxMat = nullptr; // Free the skybox map

		// Free all the maps and textures
		s_irradianceMap = nullptr;
		s_prefilterMap = nullptr;
		s_brdfLUT = nullptr;
	}

	void PBR::Update()
	{
		auto scene = Scene::CurrentScene();
		if (!scene)
			return;

		auto&& skyboxes = scene->GetComponents<SkyboxComponent>();
		if (skyboxes.size() == 0) // A skybox is present
			return;
		
		auto& skybox = skyboxes[0].second;

		if (!skybox.material || !skybox.material->IsValid()) // The skybox is valid
			return;

		// Check if the skybox changed
		if (s_skyboxMat != skybox.material)
		{
			s_skyboxMat = skybox.material;
			// Reload the maps
			auto& skyboxMap = std::get<Asset<Cubemap>>(skybox.material->GetUniform("skybox"));
			
			if (skyboxMap) // The skybox has a cubemap
			{
				s_irradianceMap = PBR::CreateIrradianceMap(*(std::shared_ptr<Cubemap>)skyboxMap, IrradianceSize, IrradianceSampleDelta);
				s_prefilterMap = PBR::CreatePreFilterMap(*(std::shared_ptr<Cubemap>)skyboxMap, PrefilterSize);

				RenderApi::SetActiveTexture(s_irradianceMapSlot);
				s_irradianceMap->Bind();

				RenderApi::SetActiveTexture(s_prefilterMapSlot);
				s_prefilterMap->Bind();

				RenderApi::SetActiveTexture(0);
			}
		}
	}
}
