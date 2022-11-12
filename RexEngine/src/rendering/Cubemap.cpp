#include <REPch.h>
#include "Cubemap.h"

#include <stb/stb_image.h>

#include "Texture.h"
#include "Shader.h"
#include "FrameBuffer.h"
#include "Shapes.h"

namespace RexEngine
{
	namespace Internal
	{
		constexpr int TextureFormatToSTBInt(RenderApi::PixelFormat format)
		{
			switch (format)
			{
			case RenderApi::PixelFormat::RGB:
				return 3;
			case RenderApi::PixelFormat::RGBA:
				return 4;
			}

			RE_ASSERT(false, "Invalid texture format");
			return -1;
		}

	}

	Cubemap::Cubemap()
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

	Cubemap::~Cubemap()
	{
		RenderApi::DeleteTexture(m_id);
	}

	std::shared_ptr<Cubemap> Cubemap::FromFiles(const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back, RenderApi::PixelFormat format)
	{
		const std::array<std::string, 6> paths = { right, left, top, bottom, front, back };
		auto cubemap = std::make_shared<Cubemap>();

		for (int i = 0; i < 6; i++)
		{
			Vector2Int size;
			int nbChannels;
			unsigned char* data = stbi_load(paths[i].c_str(), &size.x, &size.y, &nbChannels, Internal::TextureFormatToSTBInt(format));

			if (data)
				RenderApi::SetCubemapFace(cubemap->GetId(), (RenderApi::CubemapFace)i, format, size, data, format, RenderApi::PixelType::UByte);
			else
				RE_LOG_ERROR("Error reading cubemap texture at {}", paths[i]);

			stbi_image_free(data);
		}

		return cubemap;
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