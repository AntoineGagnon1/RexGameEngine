#pragma once

#include "RenderApi.h"

namespace RexEngine
{
	class RenderBuffer
	{
	public:
		RenderBuffer(RenderApi::PixelType type, Vector2Int size)
		{
			m_id = RenderApi::MakeRenderBuffer(type, size);
		}

		~RenderBuffer()
		{
			RenderApi::DeleteRenderBuffer(m_id);
		}

		RenderBuffer(const RenderBuffer&) = delete;

		void Bind() const
		{
			RenderApi::BindRenderBuffer(m_id);
		}

		static void UnBind()
		{
			RenderApi::BindRenderBuffer(RenderApi::InvalidBufferID);
		}

		RenderApi::BufferID GetId() const { return m_id; }

	private:
		RenderApi::BufferID m_id;
	};
}