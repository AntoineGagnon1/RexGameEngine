#pragma once

#include <array>
#include <string>

#include "RenderApi.h"

namespace RexEngine
{
	class Cubemap
	{
	public:

		// TODO : rename to FromFiles()
		explicit Cubemap(const std::string& right, const std::string& left,
			const std::string& top, const std::string& bottom,
			const std::string& front, const std::string& back, RenderApi::PixelFormat format);
		Cubemap();
		~Cubemap();
		
		// Project a HDRI file on a cubemap
		// Size : size of each face of the final cubemap
		static std::shared_ptr<Cubemap> FromHDRI(const std::string& path, Vector2Int size);
		
		// Sample delta : accuracy of the result
		static std::shared_ptr<Cubemap> CreateIrradianceMap(const Cubemap& from, Vector2Int size, float sampleDelta = 0.025f);

		RenderApi::TextureID GetId() const { return m_id; }

		void Bind() const { RenderApi::BindTexture(m_id, RenderApi::TextureTarget::Cubemap); }
		static void UnBind() { RenderApi::BindTexture(RenderApi::InvalidTextureID, RenderApi::TextureTarget::Cubemap); }

	private:
		void SetDefaultTextureOptions();

	private:
		RenderApi::TextureID m_id;

	};
}