#pragma once

#include <string>
#include <span>
#include <unordered_map>

#include "../math/Matrix.h"

namespace RexEngine
{
	class RenderApi
	{
	public:

		// Init, will be called by Libs.cpp automatically
		static void Init();

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

		// <name, location>, todo : return type
		static std::unordered_map<std::string, int> GetShaderUniforms(ShaderID id);
		static void SetUniformMatrix4(int location, const Matrix4& matrix);
		static void SetUniformVector3(int location, const Vector3& vec);
		static void SetUniformFloat(int location, float value);
		static void SetUniformInt(int location, int value);

		// TODO : get attributes : https://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade

		// Buffers
		typedef unsigned int BufferID;
		inline static constexpr BufferID InvalidBufferID = 0;
		enum class BufferType { Vertex, Indice, Uniforms };
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

		static void SubBufferData(BufferID id, BufferType type, size_t offset, size_t size, const void* data);
		static void BindBufferBase(BufferID id, BufferType type, int location);

		// Vertex Attributes
		typedef unsigned int VertexAttribID;
		enum class VertexAttributeType { Float, Float2, Float3, Float4 };

		// <VertexAttributeType, int location>
		static VertexAttribID MakeVertexAttributes(std::span<std::tuple<VertexAttributeType, int>> attributes, BufferID vertexBuffer, BufferID indices);
		static void DeleteVertexAttributes(VertexAttribID id);
		static void BindVertexAttributes(VertexAttribID id);

		// Textures
		typedef unsigned int TextureID;
		inline static constexpr TextureID InvalidTextureID = 0;

		enum class TextureTarget { Texture2D, Cubemap };
		
		// Order is important (used in loops) :
		enum class CubemapFace { CubemapRight = 0, CubemapLeft = 1, CubemapTop = 2, CubemapBottom = 3, CubemapFront = 4, CubemapBack = 5 };

		enum class PixelFormat { RGB, RGBA, Depth, RGB16F };
		enum class PixelType { UByte, Depth, Float };


		enum class TextureOption { WrapS, WrapT, WrapR, MinFilter, MagFilter };
		enum class TextureOptionValue { Repeat, ClampToEdge, Linear };

		static TextureID MakeTexture(TextureTarget target, PixelFormat gpuFormat, Vector2Int size, const void* data, PixelFormat dataFormat, PixelType dataType);
		static void BindTexture(TextureID id, TextureTarget target);
		static void SetTextureOption(TextureID id, TextureTarget target, TextureOption option, TextureOptionValue value);
		static void DeleteTexture(TextureID id);

		static TextureID MakeCubemap();
		static void SetCubemapFace(TextureID id, CubemapFace face, PixelFormat gpuFormat, Vector2Int size, const void* data, PixelFormat dataFormat, PixelType dataType);

		static int GetActiveTexture();
		static void SetActiveTexture(int index);

		// Viewport
		static void SetViewportSize(Vector2Int size);
		static Vector2Int GetViewportSize();
		static void ClearColorBit(); // TODO : Color
		static void ClearDepthBit();

		// Drawing
		// Needs a shader and a VertexAttribute to be bound first
		static void DrawElements(size_t count);

		// Culling
		enum class CullingMode : unsigned char { Front /*Will show front faces only*/, Back /*Will show back faces only*/, Both /*Will show all faces*/ };
		static void SetCullingMode(CullingMode mode);

		// Depth test
		enum class DepthFunction { Less, LessEqual, Greater, GreaterEqual };
		static void SetDepthFunction(DepthFunction function);
	
		// Render buffers
		static BufferID MakeRenderBuffer(PixelType type, Vector2Int size);
		static void BindRenderBuffer(BufferID id);
		static void DeleteRenderBuffer(BufferID id);
			
		// Frame buffers
		typedef unsigned int FrameBufferID;
		inline static constexpr TextureID InvalidFrameBufferID = 0;
		
		enum class FrameBufferTextureType { Color, Depth };

		static FrameBufferID MakeFrameBuffer();
		static void BindFrameBuffer(FrameBufferID id);
		static void DeleteFrameBuffer(FrameBufferID id);
		static void BindFrameBufferTexture(FrameBufferID id, TextureID textureID, FrameBufferTextureType type);
		static void BindFrameBufferRenderBuffer(FrameBufferID id, BufferID renderBufferID, FrameBufferTextureType type);
		static void BindFrameBufferCubemapFace(FrameBufferID id, CubemapFace face, TextureID cubemap, FrameBufferTextureType type);
	};
}