#pragma once

#include "../math/Vectors.h"
#include "RenderApi.h"
#include "Texture.h"
#include "RenderBuffer.h"
#include "Cubemap.h"

namespace RexEngine
{
	class FrameBuffer
	{
	public:
		FrameBuffer()
		{
			m_id = RenderApi::MakeFrameBuffer();
		}

		~FrameBuffer()
		{
			RenderApi::DeleteFrameBuffer(m_id);
		}

		FrameBuffer(const FrameBuffer&) = delete;

		void Bind() const
		{
			RenderApi::BindFrameBuffer(m_id);
		}

		inline static void UnBind()
		{
			RenderApi::BindFrameBuffer(RenderApi::InvalidFrameBufferID);
		}

		void BindTexture(const Texture& texture, RenderApi::FrameBufferTextureType type)
		{
			RenderApi::BindFrameBufferTexture(m_id, texture.GetId(), type);
		}

		void BindRenderBuffer(const RenderBuffer& buffer, RenderApi::FrameBufferTextureType type)
		{
			RenderApi::BindFrameBufferRenderBuffer(m_id, buffer.GetId(), type);
		}

		void BindCubemapFace(const Cubemap& cubemap, RenderApi::CubemapFace face, RenderApi::FrameBufferTextureType type)
		{
			RenderApi::BindFrameBufferCubemapFace(m_id, face, cubemap.GetId(), type);
		}

	private:
		RenderApi::FrameBufferID m_id;
	};
}