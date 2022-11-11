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

		RenderApi::TextureID GetId() const { return m_id; }

		void Bind() const { RenderApi::BindTexture(m_id, RenderApi::TextureTarget::Cubemap); }
		static void UnBind() { RenderApi::BindTexture(RenderApi::InvalidTextureID, RenderApi::TextureTarget::Cubemap); }

	private:
		RenderApi::TextureID m_id;

	};
}