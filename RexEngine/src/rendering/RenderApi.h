#pragma once

#include <string>
#include <span>

namespace RexEngine
{
	class RenderApi
	{
	public:

		// Shaders
		typedef unsigned int ShaderID;
		inline static constexpr ShaderID InvalidShaderID = 0;
		enum class ShaderType { Vertex = 0, Fragment = 1 };

		static ShaderID CompileShader(const std::string& source, ShaderType type);
		static ShaderID LinkShaders(ShaderID vertex, ShaderID fragment);
		static void DeleteShader(ShaderID id);
		static void DeleteLinkedShader(ShaderID id);
		static ShaderID GetFallbackShader(); // A default all pink shader
		static void BindShader(ShaderID id);


		// Buffers
		typedef unsigned int BufferID;
		inline static constexpr BufferID InvalidBufferID = 0;
		enum class BufferType { Vertex, Indice };
		enum class BufferMode { Static, Dynamic };

		static BufferID MakeBuffer();
		static void BindBuffer(BufferID id, BufferType type);
		static void DeleteBuffer(BufferID id);
		// Length is in bytes
		static void SetBufferData(BufferID id, BufferType type, BufferMode mode, const uint8_t* data, size_t length);
		template<typename T>
		static void SetBufferData(BufferID id, BufferType type, BufferMode mode, std::span<T> data)
		{ 
			SetBufferData(id, type, mode, (uint8_t*)data.data(), data.size_bytes());
		}

		// Vertex Attributes
		typedef unsigned int VertexAttribID;
		enum class VertexAttributeType { Float, Float2, Float3, Float4 };

		// <VertexAttributeType, int location>
		static VertexAttribID MakeVertexAttributes(std::span<std::tuple<VertexAttributeType, int>> attributes, BufferID vertexBuffer, BufferID indices);
		static void DeleteVertexAttributes(VertexAttribID id);
		static void BindVertexAttributes(VertexAttribID id);

		// Viewport
		static void SetViewportSize(Vector2Int size);
		static void ClearColorBit(); // TODO : Color

		// Drawing
		// Needs a shader and a VertexAttribute to be bound first
		static void DrawElements(size_t count);
	};
}