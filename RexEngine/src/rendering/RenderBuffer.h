#pragma once

#include "RenderApi.h"

namespace RexEngine
{
	class RenderBuffer
	{
	public:
		RenderBuffer(RenderApi::PixelType type, Vector2Int size)
			: m_pixelType(type)
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

		void SetSize(Vector2Int newSize)
		{
			RenderApi::SetRenderBufferSize(m_id, m_pixelType, newSize);
		}

		RenderApi::BufferID GetId() const { return m_id; }

	private:
		RenderApi::BufferID m_id;
		RenderApi::PixelType m_pixelType;
	};
}