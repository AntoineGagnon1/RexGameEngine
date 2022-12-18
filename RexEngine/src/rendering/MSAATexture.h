#pragma once
#include "RenderApi.h"

namespace RexEngine
{
	// Multisampled texture
	class MSAATexture
	{
	public:
		MSAATexture(RenderApi::PixelFormat gpuFormat, Vector2Int size, int sampleCount, RenderApi::TextureTarget target = RenderApi::TextureTarget::Texture2D_Multisample)
			: m_size(size), m_gpuFormat(gpuFormat), m_sampleCount(sampleCount), m_target(target)
		{
			m_id = RenderApi::MakeTextureMultisampled(m_target, gpuFormat, m_size, sampleCount);
		}

		~MSAATexture()
		{
			RenderApi::DeleteTexture(m_id);
		}

		MSAATexture(const MSAATexture&) = delete;

		void SetSize(Vector2Int newSize)
		{
			m_size = newSize;
			RenderApi::SetTextureDataMultisampled(m_id, m_target, m_gpuFormat, newSize, m_sampleCount);
		}

		int Width() const { return m_size.x; }
		int Height() const { return m_size.y; }

		Vector2Int Size() const { return m_size; }

		RenderApi::TextureID GetId() const { return m_id; }

		void Bind() const
		{
			RenderApi::BindTexture(m_id, m_target);
		}

		void UnBind() const
		{
			RenderApi::BindTexture(RenderApi::InvalidTextureID, m_target);
		}
	private:
		Vector2Int m_size;
		RenderApi::TextureID m_id;
		RenderApi::TextureTarget m_target;
		RenderApi::PixelFormat m_gpuFormat; // Cached for SetData()
		int m_sampleCount;
	};
}