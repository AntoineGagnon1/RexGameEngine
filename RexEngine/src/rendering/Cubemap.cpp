#include <REPch.h>
#include "Cubemap.h"

#include <stb/stb_image.h>

#include "FrameBuffer.h"
#include "Shader.h"
#include "Shapes.h"

namespace RexEngine::Internal
{
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
	FragColor = vec4(color, 1.0f);
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

namespace RexEngine
{
	Cubemap::Cubemap()
		: m_source(Asset<Texture>()), m_mode(ProjectionMode::HDRI), m_size(0)
	{
		m_id = RenderApi::MakeCubemap();

		using Option = RenderApi::TextureOption;
		using Value = RenderApi::TextureOptionValue;
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::MagFilter, Value::Linear);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::MinFilter, Value::Linear);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::WrapS, Value::ClampToEdge);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::WrapT, Value::ClampToEdge);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::WrapR, Value::ClampToEdge);
	}

	Cubemap::Cubemap(Asset<Texture> source, int size, ProjectionMode mode)
		: m_source(source), m_mode(mode), m_size(size)
	{
		m_id = RenderApi::MakeCubemap();

		if (!source)
			return;

		if (mode == ProjectionMode::HDRI)
		{
			// Init the cubemap with empty textures
			for (int i = 0; i < 6; i++)
			{
				RenderApi::SetCubemapFace(m_id, (RenderApi::CubemapFace)i, RenderApi::PixelFormat::RGB16F, {size , size},
					NULL, RenderApi::PixelFormat::RGB, RenderApi::PixelType::Float);
			}

			Matrix4 projectionMatrix = Matrix4::MakePerspective(90.0f, 1.0f, 0.1f, 10.0f);

			// Make the framebuffer
			RenderApi::FrameBufferID oldFrameBuffer = RenderApi::GetBoundFrameBuffer();
			FrameBuffer frameBuffer;
			RenderBuffer renderBuffer(RenderApi::PixelType::Depth, { size , size });
			frameBuffer.BindRenderBuffer(renderBuffer, RenderApi::FrameBufferTextureType::Depth);

			// Load the shader
			static NoDestroy<Shader> projectionShader(Internal::vertexShaderString + Internal::projectionShaderString, RenderApi::CullingMode::Back);

			// Set the texture for the shader
			projectionShader->Bind();
			projectionShader->SetUniformInt("equirectangularMap", 0);
			RenderApi::SetActiveTexture(0);
			source->Bind();

			// Set the viewport to the size of the cubemap
			Vector2Int oldViewportSize = RenderApi::GetViewportSize(); // Cache the size to revert at the end
			RenderApi::SetViewportSize({ size , size });

			frameBuffer.Bind();
			RenderApi::SetDepthFunction(RenderApi::DepthFunction::LessEqual);
			auto cubeMesh = Shapes::GetCubeMesh();
			cubeMesh->Bind(); // Bind the cube

			for (unsigned int i = 0; i < 6; ++i)
			{
				projectionShader->SetUniformMatrix4("transformMatrix", projectionMatrix *Internal::ViewMatrices[i]);

				frameBuffer.BindCubemapFace(*this, (RenderApi::CubemapFace)i, RenderApi::FrameBufferTextureType::Color);

				RenderApi::ClearColorBit();
				RenderApi::ClearDepthBit();

				RenderApi::DrawElements(cubeMesh->GetIndexCount()); // Render to the cubemap
			}

			RenderApi::BindFrameBuffer(oldFrameBuffer);
			RenderApi::SetViewportSize(oldViewportSize); // Revert to the cached viewport size

			GenerateMipmaps();
		}

		using Option = RenderApi::TextureOption;
		using Value = RenderApi::TextureOptionValue;
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::MagFilter, Value::Linear);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::MinFilter, Value::Linear);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::WrapS, Value::ClampToEdge);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::WrapT, Value::ClampToEdge);
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, Option::WrapR, Value::ClampToEdge);
	}

	Cubemap::~Cubemap()
	{
		RenderApi::DeleteTexture(m_id);
	}

	void Cubemap::SetOption(RenderApi::TextureOption option, RenderApi::TextureOptionValue value)
	{
		RenderApi::SetTextureOption(m_id, RenderApi::TextureTarget::Cubemap, option, value);
	}

	void Cubemap::GenerateMipmaps()
	{ 
		Bind();
		RenderApi::GenerateMipmaps(RenderApi::TextureTarget::Cubemap);
	}
}