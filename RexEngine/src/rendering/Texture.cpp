#include <REPch.h>
#include "Texture.h"

#include <stb/stb_image.h>

namespace RexEngine
{
	namespace Internal
	{
		constexpr RenderApi::PixelFormat NbChannelsToGL(int nbChannels)
		{
			switch (nbChannels)
			{
			case 3:
				return RenderApi::PixelFormat::RGB;
			case 4:
				return RenderApi::PixelFormat::RGBA;
			}

			RE_ASSERT(false, "Invalid texture format");
			return (RenderApi::PixelFormat)-1;
		}
	}

	Texture::Texture(RenderApi::TextureTarget target, RenderApi::PixelFormat gpuFormat, Vector2Int size, const void* data, RenderApi::PixelFormat dataFormat, RenderApi::PixelType dataType)
		: m_size(size), m_target(target), m_gpuFormat(gpuFormat)
	{
		m_id = RenderApi::MakeTexture(target, gpuFormat, m_size, data, dataFormat, dataType);

		// Default texture options
		using Option = RenderApi::TextureOption;
		using Value = RenderApi::TextureOptionValue;
		SetOption(Option::WrapS, Value::Repeat);
		SetOption(Option::WrapT, Value::Repeat);
		SetOption(Option::MinFilter, Value::Linear);
		SetOption(Option::MagFilter, Value::Linear);
	}

	Texture::~Texture()
	{
		RenderApi::DeleteTexture(m_id);
	}


	std::shared_ptr<Texture> Texture::FromFile(const std::string& path, RenderApi::TextureTarget target, RenderApi::PixelFormat gpuFormat)
	{
		int nbChannels;
		Vector2Int size;
		
		stbi_set_flip_vertically_on_load(false);
		unsigned char* data = stbi_load(path.c_str(), &size.x, &size.y, &nbChannels, 0);

		if (data)
			return std::make_shared<Texture>(target, gpuFormat, size, data, Internal::NbChannelsToGL(nbChannels), RenderApi::PixelType::UByte);
		else
			RE_LOG_ERROR("Error reading texture at {}", path);

		stbi_image_free(data);

		return std::shared_ptr<Texture>();
	}

	std::shared_ptr<Texture> Texture::FromHDRIFile(const std::string& path, RenderApi::TextureTarget target, RenderApi::PixelFormat gpuFormat)
	{
		int nbChannels;
		Vector2Int size;

		stbi_set_flip_vertically_on_load(true);
		float* data = stbi_loadf(path.c_str(), &size.x, &size.y, &nbChannels, 0);

		if (data)
			return std::make_shared<Texture>(target, gpuFormat, size, data, Internal::NbChannelsToGL(nbChannels), RenderApi::PixelType::Float);
		else
			RE_LOG_ERROR("Error reading hdri file at {}", path);

		stbi_image_free(data);

		return std::shared_ptr<Texture>();
	}

	void Texture::SetData(Vector2Int newSize, const void* data, RenderApi::PixelFormat dataFormat, RenderApi::PixelType dataType)
	{
		m_size = newSize;
		RenderApi::SetTextureData(m_id, m_target, m_gpuFormat, newSize, data, dataFormat, dataType);
	}

	void Texture::SetOption(RenderApi::TextureOption option, RenderApi::TextureOptionValue value)
	{
		RenderApi::SetTextureOption(m_id, m_target, option, value);
	}


	void Texture::Bind() const
	{
		RenderApi::BindTexture(m_id, m_target);
	}

	void Texture::UnBind() const
	{
		RenderApi::BindTexture(RenderApi::InvalidTextureID, m_target);
	}
}