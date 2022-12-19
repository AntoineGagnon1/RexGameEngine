#include "REPch.h"
#include "PBR.h"

#include "Texture.h"
#include "FrameBuffer.h"
#include "RenderBuffer.h"
#include "Shader.h"
#include "Shapes.h"

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
out vec2 FragColor;
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
    FragColor = integratedBRDF;
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

    std::shared_ptr<Cubemap> RexEngine::PBR::FromHDRI(const std::string& path, Vector2Int size)
    {
		std::ifstream file(path);
		auto hdrMap = Texture::FromHDRStream2D(file, RenderApi::PixelFormat::RGB16F, true);
		auto cubemap = std::make_shared<Cubemap>();
		auto cubemapId = cubemap->GetId();

		if (hdrMap)
		{
			// Init the cubemap with empty textures
			for (int i = 0; i < 6; i++)
			{
				RenderApi::SetCubemapFace(cubemapId, (RenderApi::CubemapFace)i, RenderApi::PixelFormat::RGB16F, size,
					NULL, RenderApi::PixelFormat::RGB, RenderApi::PixelType::Float);
			}

			Matrix4 projectionMatrix = Matrix4::MakePerspective(90.0f, (float)size.x / (float)size.y, 0.1f, 10.0f);

			// Make the framebuffer
			RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
			FrameBuffer frameBuffer;
			RenderBuffer renderBuffer(RenderApi::PixelType::Depth, size);
			frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

			// Load the shader
			static NoDestroy<Shader> projectionShader(Internal::vertexShaderString + Internal::projectionShaderString);

			// Set the texture for the shader
			projectionShader->SetUniformInt("equirectangularMap", 0);
			RenderApi::SetActiveTexture(0);
			hdrMap->Bind();

			// Set the viewport to the size of the cubemap
			Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end
			RenderApi::SetViewportSize(size);

			frameBuffer.Bind();

			auto cubeMesh = Shapes::GetCubeMesh();
			cubeMesh->Bind(); // Bind the cube

			for (unsigned int i = 0; i < 6; ++i)
			{
				projectionShader->SetUniformMatrix4("transformMatrix", projectionMatrix * Internal::ViewMatrices[i]);

				frameBuffer.BindCubemapFace(*cubemap, (RenderApi::CubemapFace)i, RenderApi::FrameBufferTextureType::Color);

				RenderApi::ClearColorBit();
				RenderApi::ClearDepthBit();

				RenderApi::DrawElements(cubeMesh->GetIndexCount()); // Render to the cubemap
			}

			RenderApi::BindFrameBuffer(oldFrameBuffer);
			RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size
		}

		cubemap->GenerateMipmaps();

		return cubemap;
    }


	std::shared_ptr<Cubemap> PBR::CreateIrradianceMap(const Cubemap& from, Vector2Int size, float sampleDelta)
	{
		auto cubemap = std::make_shared<Cubemap>();
		auto cubemapId = cubemap->GetId();

		// Init the cubemap with empty textures
		for (int i = 0; i < 6; i++)
		{
			RenderApi::SetCubemapFace(cubemapId, (RenderApi::CubemapFace)i, RenderApi::PixelFormat::RGB16F, size,
				NULL, RenderApi::PixelFormat::RGB, RenderApi::PixelType::Float);
		}

		Matrix4 projectionMatrix = Matrix4::MakePerspective(90.0f, (float)size.x / (float)size.y, 0.1f, 10.0f);

		// Create the frame buffer
		RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
		FrameBuffer frameBuffer;
		RenderBuffer renderBuffer(RenderApi::PixelType::Depth, size);
		frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

		// Create the irradiance shader
		static NoDestroy<Shader> irradianceShader(Internal::vertexShaderString + Internal::irradianceShaderString);

		irradianceShader->SetUniformFloat("sampleDelta", sampleDelta);
		irradianceShader->SetUniformInt("environmentMap", 0);
		RenderApi::SetActiveTexture(0);
		from.Bind();

		Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end
		RenderApi::SetViewportSize(size);

		frameBuffer.Bind();

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

	std::shared_ptr<Cubemap> PBR::CreatePreFilterMap(const Cubemap& from, Vector2Int size)
	{
		auto cubemap = std::make_shared<Cubemap>();
		auto cubemapId = cubemap->GetId();

		// Init the cubemap with empty textures
		for (int i = 0; i < 6; i++)
		{
			RenderApi::SetCubemapFace(cubemapId, (RenderApi::CubemapFace)i, RenderApi::PixelFormat::RGB16F, size,
				NULL, RenderApi::PixelFormat::RGB, RenderApi::PixelType::Float);
		}

		cubemap->SetOption(RenderApi::TextureOption::MinFilter, RenderApi::TextureOptionValue::LinearMipmap);
		cubemap->GenerateMipmaps();

		Matrix4 projectionMatrix = Matrix4::MakePerspective(90.0f, (float)size.x / (float)size.y, 0.1f, 10.0f);

		// Create the frame buffer
		RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
		FrameBuffer frameBuffer;
		RenderBuffer renderBuffer(RenderApi::PixelType::Depth, size);
		frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

		// Create the irradiance shader
		static NoDestroy<Shader> prefilterShader(Internal::vertexShaderString + Internal::prefilterShaderString);

		prefilterShader->SetUniformInt("environmentMap", 0);
		RenderApi::SetActiveTexture(0);
		from.Bind();

		Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end

		frameBuffer.Bind();

		auto cubeMesh = Shapes::GetCubeMesh();
		cubeMesh->Bind(); // Bind the cube

		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; mip++)
		{
			// Resize framebuffer according to mip level
			unsigned int mipWidth = (unsigned int)(size.x * std::pow(0.5, mip));
			unsigned int mipHeight = (unsigned int)(size.y * std::pow(0.5, mip));

			//renderBuffer.Bind();
			//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
			RenderApi::SetViewportSize(Vector2Int(mipWidth, mipHeight));

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

		// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
		RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
		FrameBuffer frameBuffer;
		RenderBuffer renderBuffer(RenderApi::PixelType::Depth, size);
		frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);
		frameBuffer.BindTexture(*texture, RenderApi::FrameBufferTextureType::Color);

		Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end
		RenderApi::SetViewportSize(size);

		static NoDestroy<Shader> shader(Internal::lutShader);

		shader->Bind();
		RenderApi::ClearColorBit();
		RenderApi::ClearDepthBit();

		auto quad = Shapes::GetQuadMesh();
		quad->Bind();
		RenderApi::DrawElements(quad->GetIndexCount());

		RenderApi::BindFrameBuffer(oldFrameBuffer);
		RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size

		return texture;
	}
}
