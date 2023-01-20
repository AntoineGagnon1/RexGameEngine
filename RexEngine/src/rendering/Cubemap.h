#pragma once

#include <array>
#include <string>

#include "RenderApi.h"
#include "Texture.h"
#include "../assets/AssetManager.h"

namespace RexEngine
{
	// A cubemap asset
	// cubemap can be created using a texture and a projection mode
	class Cubemap
	{
	public:

		enum class ProjectionMode { HDRI }; // TODO : add Faces

		// An empty cubemap
		Cubemap();
		Cubemap(Asset<Texture> source, int size, ProjectionMode mode);
		~Cubemap();
		
		Cubemap(const Cubemap&) = delete;

		Asset<Texture> GetSource() const { return m_source; }
		int GetSize() const { return m_size; }
		auto GetMode() const { return m_mode; }
		RenderApi::TextureID GetId() const { return m_id; }

		void Bind() const { RenderApi::BindTexture(m_id, RenderApi::TextureTarget::Cubemap); }
		static void UnBind() { RenderApi::BindTexture(RenderApi::InvalidTextureID, RenderApi::TextureTarget::Cubemap); }

		void SetOption(RenderApi::TextureOption option, RenderApi::TextureOptionValue value);
		
		void GenerateMipmaps();

		template<typename Archive>
		static std::shared_ptr<Cubemap> LoadFromAssetFile([[maybe_unused]] Guid assetGuid, [[maybe_unused]] Archive& metaDataArchive, std::istream& assetFile)
		{
			JsonDeserializer archive(assetFile);
			Asset<Texture> source;
			int mode, size;
			archive(CUSTOM_NAME(source, "Source"),
				CUSTOM_NAME(mode, "Mode"),
				CUSTOM_NAME(size, "Size"));

			return std::make_shared<Cubemap>(source, size, (ProjectionMode)mode);
		}

		template<typename Archive>
		void SaveToAssetFile([[maybe_unused]] Archive& metaDataArchive, std::ostream& assetFile)
		{
			JsonSerializer archive(assetFile);
			archive(CUSTOM_NAME(m_source, "Source"),
				CUSTOM_NAME((int)m_mode, "Mode"),
				CUSTOM_NAME(m_size, "Size"));
		}

	private:
		RenderApi::TextureID m_id;

		Asset<Texture> m_source;
		ProjectionMode m_mode;
		int m_size;
	};
}