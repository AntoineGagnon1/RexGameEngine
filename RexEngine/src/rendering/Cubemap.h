#pragma once

#include <array>
#include <string>

#include "RenderApi.h"

namespace RexEngine
{
	class Cubemap
	{
	public:

		Cubemap();
		~Cubemap();
		
		Cubemap(const Cubemap&) = delete;

		static std::shared_ptr<Cubemap> FromFiles(const std::string& right, const std::string& left,
			const std::string& top, const std::string& bottom,
			const std::string& front, const std::string& back, RenderApi::PixelFormat format);

		//static std::shared_ptr<Cubemap> FromTextures()

		RenderApi::TextureID GetId() const { return m_id; }

		void Bind() const { RenderApi::BindTexture(m_id, RenderApi::TextureTarget::Cubemap); }
		static void UnBind() { RenderApi::BindTexture(RenderApi::InvalidTextureID, RenderApi::TextureTarget::Cubemap); }

		void SetOption(RenderApi::TextureOption option, RenderApi::TextureOptionValue value);
		
		void GenerateMipmaps();

		template<typename Archive>
		static std::shared_ptr<Cubemap> LoadFromAssetFile(Guid assetGuid, const Archive& metaDataArchive, std::istream& assetFile)
		{
			return nullptr;
		}

		template<typename Archive>
		void SaveToAssetFile(Archive& metaDataArchive) const
		{
		}

	private:
		RenderApi::TextureID m_id;

	};
}