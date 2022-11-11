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
		const std::string projectionShaderString = R"(
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
		auto hdrMap = Texture::FromHDRIFile(path, RenderApi::TextureTarget::Texture2D, RenderApi::PixelFormat::RGB16F);
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
			FrameBuffer frameBuffer;
			RenderBuffer renderBuffer(RenderApi::PixelType::Depth, size);
			frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

			// Load the shader
			static NoDestroy<Shader> projectionShader(Internal::projectionShaderString);

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

			frameBuffer.UnBind();
			RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size
		}

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
		FrameBuffer frameBuffer;
		RenderBuffer renderBuffer(RenderApi::PixelType::Depth, size);
		frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

		// Create the irradiance shader
		static NoDestroy<Shader> irradianceShader(Internal::irradianceShaderString);

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
		frameBuffer.UnBind();
		RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size

		return cubemap;
	}
}
