#pragma once

#include "../math/Vectors.h"
#include "RenderApi.h"
#include "Texture.h"
#include "MSAATexture.h"
#include "RenderBuffer.h"
#include "Cubemap.h"

namespace RexEngine
{
	enum class FramBufferTarget { Read, Draw, ReadDraw };

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

		auto GetID() const { return m_id; }

		void Bind(FramBufferTarget target = FramBufferTarget::ReadDraw) const
		{
			switch (target)
			{
			case RexEngine::FramBufferTarget::Read:
				RenderApi::BindFrameBufferRead(m_id);
				break;
			case RexEngine::FramBufferTarget::Draw:
				RenderApi::BindFrameBufferDraw(m_id);
				break;
			case RexEngine::FramBufferTarget::ReadDraw:
				RenderApi::BindFrameBuffer(m_id);
				break;
			}
		}

		inline static void UnBind(FramBufferTarget target = FramBufferTarget::ReadDraw)
		{
			switch (target)
			{
			case RexEngine::FramBufferTarget::Read:
				RenderApi::BindFrameBufferRead(RenderApi::InvalidFrameBufferID);
				break;
			case RexEngine::FramBufferTarget::Draw:
				RenderApi::BindFrameBufferDraw(RenderApi::InvalidFrameBufferID);
				break;
			case RexEngine::FramBufferTarget::ReadDraw:
				RenderApi::BindFrameBuffer(RenderApi::InvalidFrameBufferID);
				break;
			}
		}

		void BindTexture(const Texture& texture, RenderApi::FrameBufferTextureType type)
		{
			RenderApi::BindFrameBufferTexture(m_id, texture.GetId(), type);
		}

		void BindTexture(const MSAATexture& texture, RenderApi::FrameBufferTextureType type)
		{
			RenderApi::BindFrameBufferTextureMultisampled(m_id, texture.GetId(), type);
		}

		void BindRenderBuffer(const RenderBuffer& buffer, RenderApi::FrameBufferTextureType type)
		{
			RenderApi::BindFrameBufferRenderBuffer(m_id, buffer.GetId(), type);
		}

		void UnBindRenderBuffer(RenderApi::FrameBufferTextureType type)
		{
			RenderApi::BindFrameBufferRenderBuffer(m_id, RenderApi::InvalidBufferID, type);
		}

		void BindCubemapFace(const Cubemap& cubemap, RenderApi::CubemapFace face, RenderApi::FrameBufferTextureType type, int mip = 0)
		{
			RenderApi::BindFrameBufferCubemapFace(m_id, face, cubemap.GetId(), type, mip);
		}
		
		void BlitInto(const FrameBuffer& into, Vector2Int inSize, Vector2Int outSize, RenderApi::FrameBufferTextureType type)
		{
			auto oldRead = RenderApi::GetBoundReadFrameBuffer();
			auto oldDraw = RenderApi::GetBoundDrawFrameBuffer();

			Bind(RexEngine::FramBufferTarget::Read);
			into.Bind(RexEngine::FramBufferTarget::Draw);

			RenderApi::BlitFrameBuffer({ 0,0 }, inSize, {0,0}, outSize, type);

			RenderApi::BindFrameBufferRead(oldRead);
			RenderApi::BindFrameBufferRead(oldDraw);
		}

	private:
		RenderApi::FrameBufferID m_id;
	};
}