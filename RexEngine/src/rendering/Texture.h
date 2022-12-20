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
		static std::shared_ptr<Texture> FromFile(const std::filesystem::path& path);

		// Texture2D from a stream
		static std::shared_ptr<Texture> FromStream2D(std::istream& stream, RenderApi::PixelFormat gpuFormat, bool flipY);
		static std::shared_ptr<Texture> FromHDRStream2D(std::istream& stream, RenderApi::PixelFormat gpuFormat, bool flipY);


		void SetData(Vector2Int newSize, const void* data, RenderApi::PixelFormat dataFormat, RenderApi::PixelType dataType);

		int Width() const { return m_size.x; }
		int Height() const { return m_size.y; }

		Vector2Int Size() const { return m_size; }

		auto GetTarget() const { return m_target; }
		auto GetHdr() const { return m_hdr; }
		auto GetFlipY() const { return m_flipYOnLoad; }

		RenderApi::TextureID GetId() const { return m_id; }
		RenderApi::PixelFormat GetFormat() const { return m_gpuFormat; }

		void SetOption(RenderApi::TextureOption option, RenderApi::TextureOptionValue value);

		// Warning the texture will be in an invalid state after this, reload the asset to make it valid again
		void ChangeSettings(RenderApi::TextureTarget newTarget, RenderApi::PixelFormat newGpuFormat, bool newFlipYOnLoad, bool newHdr);

		void Bind() const;
		void UnBind() const;

		void GenerateMipmaps() const;

		template<typename Archive>
		static std::shared_ptr<Texture> LoadFromAssetFile(Guid assetGuid, Archive& metaDataArchive, std::istream& assetFile)
		{
			// First get the target
			int targetInt;
			metaDataArchive(CUSTOM_NAME(targetInt, "Target"));
			RenderApi::TextureTarget target = (RenderApi::TextureTarget)targetInt;

			if (target == RenderApi::TextureTarget::Texture2D)
			{
				Vector2Int size;
				int gpuFormat;
				bool flipY, hdr;
				metaDataArchive(CUSTOM_NAME(size, "Size"),
					CUSTOM_NAME(gpuFormat, "Format"),
					CUSTOM_NAME(hdr, "Hdr"),
					CUSTOM_NAME(flipY, "FlipY"));

				if (hdr)
					return FromHDRStream2D(assetFile, (RenderApi::PixelFormat)gpuFormat, flipY);
				else
					return FromStream2D(assetFile, (RenderApi::PixelFormat)gpuFormat, flipY);
			}

			return std::shared_ptr<Texture>();
		}

		template<typename Archive>
		void SaveToAssetFile(Archive& metaDataArchive) const
		{
			metaDataArchive(CUSTOM_NAME((int)m_target, "Target"));

			if (m_target == RenderApi::TextureTarget::Texture2D)
			{
				metaDataArchive(CUSTOM_NAME(m_size, "Size"),
					CUSTOM_NAME((int)m_gpuFormat, "Format"),
					CUSTOM_NAME(m_hdr, "Hdr"),
					CUSTOM_NAME(m_flipYOnLoad, "FlipY"));
			}
		}

		template<typename Archive>
		inline static void CreateMetaData(Archive& metaDataArchive)
		{
			// Simple texture 2D data
			metaDataArchive(CUSTOM_NAME((int)RenderApi::TextureTarget::Texture2D, "Target"),
				CUSTOM_NAME(Vector2Int(0, 0), "Size"),
				CUSTOM_NAME((int)RenderApi::PixelFormat::RGB, "Format"),
				CUSTOM_NAME(false, "Hdr"),
				CUSTOM_NAME(false , "FlipY"));
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