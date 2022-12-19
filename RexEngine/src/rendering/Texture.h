#pragma once

#include <string>

#include "../math/Vectors.h"
#include "RenderApi.h"

namespace RexEngine
{
	// .png, .hdr assets TODO : more file types
	class Texture
	{
	private:
		// Texture2D
		Texture(RenderApi::PixelFormat gpuFormat, Vector2Int size, const void* data, RenderApi::PixelFormat dataFormat, RenderApi::PixelType dataType, bool flipY, bool hdr);
	
	public:
		// Creates an empty texture
		Texture(RenderApi::PixelFormat gpuFormat, Vector2Int size) : Texture(gpuFormat, size, nullptr, RenderApi::PixelFormat::RGB, RenderApi::PixelType::UByte, false, false) {}
		~Texture();

		Texture(const Texture&) = delete;

		// A simple 2d texture, used to load icons
		inline static std::shared_ptr<Texture> FromFile(const std::filesystem::path& path)
		{
			std::ifstream file(path, std::ios::binary);

			if(file.is_open())
				return FromStream2D(file, RenderApi::PixelFormat::RGBA, false);
			else
				RE_LOG_ERROR("Error reading texture at : !", path.string());
			return std::shared_ptr<Texture>(nullptr);
		}

		// Texture2D from a stream
		static std::shared_ptr<Texture> FromStream2D(std::istream& stream, RenderApi::PixelFormat gpuFormat, bool flipY);
		static std::shared_ptr<Texture> FromHDRStream2D(std::istream& stream, RenderApi::PixelFormat gpuFormat, bool flipY);

		void SetData(Vector2Int newSize, const void* data, RenderApi::PixelFormat dataFormat, RenderApi::PixelType dataType);

		int Width() const { return m_size.x; }
		int Height() const { return m_size.y; }

		Vector2Int Size() const { return m_size; }

		RenderApi::TextureID GetId() const { return m_id; }

		void SetOption(RenderApi::TextureOption option, RenderApi::TextureOptionValue value);

		void Bind() const;
		void UnBind() const;

		template<typename Archive>
		static std::shared_ptr<Texture> LoadFromAssetFile(Guid assetGuid, Archive& metaDataArchive, std::istream& assetFile)
		{
			Vector2Int size;
			int target, gpuFormat;
			bool flipY, hdr;

			metaDataArchive(CUSTOM_NAME(hdr, "Hdr"),
				CUSTOM_NAME(size, "Size"),
				CUSTOM_NAME(target, "Target"),
				CUSTOM_NAME(gpuFormat, "GpuFormat"),
				CUSTOM_NAME(flipY, "FlipY"));

			if (hdr)
				return FromHDRStream2D(assetFile, (RenderApi::PixelFormat)gpuFormat, flipY);
			else
				return FromStream2D(assetFile, (RenderApi::PixelFormat)gpuFormat, flipY);
		}

		template<typename Archive>
		void SaveToAssetFile(Archive& metaDataArchive) const
		{
			metaDataArchive(CUSTOM_NAME(m_hdr, "Hdr"),
				CUSTOM_NAME(m_size, "Size"),
				CUSTOM_NAME((int)m_target, "Target"),
				CUSTOM_NAME((int)m_gpuFormat, "GpuFormat"),
				CUSTOM_NAME(m_flipYOnLoad, "FlipY"));
		}

	private:
		Vector2Int m_size;
		RenderApi::TextureID m_id;
		RenderApi::TextureTarget m_target; // Cache the target for SetOption()
		RenderApi::PixelFormat m_gpuFormat; // Cached for SetData()
		bool m_flipYOnLoad;
		bool m_hdr;
	};
}