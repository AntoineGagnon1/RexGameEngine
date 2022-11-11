#pragma once

#include <string>

#include "../math/Vectors.h"
#include "RenderApi.h"

namespace RexEngine
{
	class Texture
	{
	public:

		Texture(RenderApi::TextureTarget target, RenderApi::PixelFormat gpuFormat, Vector2Int size, const void* data, RenderApi::PixelFormat dataFormat, RenderApi::PixelType dataType);
		~Texture();

		// TODO : convert to asset loader
		static std::shared_ptr<Texture> FromFile(const std::string& path, RenderApi::TextureTarget target, RenderApi::PixelFormat gpuFormat);
		static std::shared_ptr<Texture> FromHDRIFile(const std::string& path, RenderApi::TextureTarget target, RenderApi::PixelFormat gpuFormat);

		int Width() const { return m_size.x; }
		int Height() const { return m_size.y; }

		Vector2Int Size() const { return m_size; }

		RenderApi::TextureID GetId() const { return m_id; }

		void SetOption(RenderApi::TextureOption option, RenderApi::TextureOptionValue value);
		void SetDefaultOptions();

		void Bind() const;
		void UnBind() const;

	private:
		Vector2Int m_size;
		RenderApi::TextureID m_id;
		RenderApi::TextureTarget m_target; // Cache the target for SetOption()
	};
}