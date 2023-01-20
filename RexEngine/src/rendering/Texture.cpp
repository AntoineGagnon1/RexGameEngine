#include <REPch.h>
#include "Texture.h"

#include <stb/stb_image.h>

#include "PBR.h"

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

		// Callbacks adapters for a stream
		constexpr stbi_io_callbacks STBICallbacks = stbi_io_callbacks{
			[](void* user, char* data, int size) { // read
				std::istream& stream = *(std::istream*)user;

				stream.read(&data[0], size);
				return (int)stream.gcount(); // long long to int is fine, max 128 bytes
			},

			[](void* user, int n) { // skip
				std::istream& stream = *(std::istream*)user;
				stream.seekg(n, std::ios::cur);
			},

			[](void* user) { // eof
				std::istream& stream = *(std::istream*)user;
				return stream.eof() ? 1 : 0; // return non-zero if at oef
			}
		};
	}

	Texture::Texture(RenderApi::PixelFormat gpuFormat, Vector2Int size, const void* data, RenderApi::PixelFormat dataFormat, RenderApi::PixelType dataType, bool flipY, bool hdr)
		: m_size(size), m_target(RenderApi::TextureTarget::Texture2D), m_gpuFormat(gpuFormat), m_flipYOnLoad(flipY), m_hdr(hdr)
	{
		m_id = RenderApi::MakeTexture(m_target, gpuFormat, m_size, data, dataFormat, dataType);

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

	std::shared_ptr<Texture> Texture::FromFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary);

		if (file.is_open())
			return FromStream2D(file, RenderApi::PixelFormat::RGBA, false);
		else
			RE_LOG_ERROR("Error reading texture at : !", path.string());
		return std::shared_ptr<Texture>(nullptr);
	}

	std::shared_ptr<Texture> Texture::FromStream2D(std::istream& stream, RenderApi::PixelFormat gpuFormat, bool flipY)
	{
		int nbChannels;
		Vector2Int size;

		stbi_set_flip_vertically_on_load(flipY);
		unsigned char* data = stbi_load_from_callbacks(&Internal::STBICallbacks, (void*)&stream, &size.x, &size.y, &nbChannels, 0);

		if (data)
			return std::shared_ptr<Texture>(new Texture(gpuFormat, size, data, Internal::NbChannelsToGL(nbChannels), RenderApi::PixelType::UByte, flipY, false));
		else
			RE_LOG_ERROR("Error reading texture !");

		stbi_image_free(data);

		return std::shared_ptr<Texture>();
	}

	std::shared_ptr<Texture> Texture::FromHDRStream2D(std::istream& stream, RenderApi::PixelFormat gpuFormat, bool flipY)
	{
		int nbChannels;
		Vector2Int size;

		stbi_set_flip_vertically_on_load(flipY);
		float* data = stbi_loadf_from_callbacks(&Internal::STBICallbacks, (void*)&stream, &size.x, &size.y, &nbChannels, 0);

		if (data)
			return std::shared_ptr<Texture>(new Texture(gpuFormat, size, data, Internal::NbChannelsToGL(nbChannels), RenderApi::PixelType::Float, flipY, true));
		else
			RE_LOG_ERROR("Error reading hdr texture !");

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

	RenderApi::TextureOptionValue Texture::GetOption(RenderApi::TextureOption option) const
	{
		return RenderApi::GetTextureOption(m_id, m_target, option);
	}

	void Texture::Bind() const
	{
		RenderApi::BindTexture(m_id, m_target);
	}

	void Texture::UnBind() const
	{
		RenderApi::BindTexture(RenderApi::InvalidTextureID, m_target);
	}

	void Texture::GenerateMipmaps() const
	{
		Bind();
		RenderApi::GenerateMipmaps(m_target);
	}

	void Texture::ChangeSettings(RenderApi::TextureTarget newTarget, RenderApi::PixelFormat newGpuFormat, bool newFlipYOnLoad, bool newHdr)
	{
		m_target = newTarget;
		m_gpuFormat = newGpuFormat;
		m_flipYOnLoad = newFlipYOnLoad;
		m_hdr = newHdr;
	}
}