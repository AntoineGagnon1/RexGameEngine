#pragma once

#include <string>
#include <span>
#include <unordered_map>
#include <typeindex>

#include "../math/Matrix.h"
#include "../core/EngineEvents.h"

namespace RexEngine
{
	class RenderApi
	{
	private:
		static void Init();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStart().Register<&RenderApi::Init>();
		});

	public:

		// Shaders
		typedef unsigned int ShaderID;
		inline static constexpr ShaderID InvalidShaderID = 0;
		enum class ShaderType { Vertex = 0, Fragment = 1 };

		static ShaderID CompileShader(const std::string& source, ShaderType type);
		static ShaderID LinkShaders(ShaderID vertex, ShaderID fragment);
		static void DeleteShader(ShaderID id);
		static void DeleteLinkedShader(ShaderID id);
		static void BindShader(ShaderID id);

		// <name, <location, type>>
		enum class UniformType { Float, Vec2, Vec3, Vec4,
								 Int, Vec2I, Vec3I, Vec4I,
								 Double, UInt, Bool,
								 Mat3, Mat4,
								 Sampler2D, SamplerCube };
		static std::unordered_map<std::string, std::tuple<int, UniformType>> GetShaderUniforms(ShaderID id);
		static void SetUniformMatrix4(int location, const Matrix4& matrix);
		static void SetUniformVector3(int location, const Vector3& vec);
		static void SetUniformFloat(int location, float value);
		static void SetUniformInt(int location, int value);

		// TODO : get attributes : https://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade

		// Buffers
		typedef unsigned int BufferID;
		inline static constexpr BufferID InvalidBufferID = 0;
		enum class BufferType { Vertex, Indice, Uniforms, ShaderStorage};
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
		static void BindBufferBase(BufferID id, int location);

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

		enum class TextureTarget { Texture2D, Cubemap, Texture2D_Multisample };
		
		// Order and values are important (used in loops) :
		enum class CubemapFace { CubemapRight = 0, CubemapLeft = 1, CubemapTop = 2, CubemapBottom = 3, CubemapFront = 4, CubemapBack = 5 };

		enum class PixelFormat { RGB, RGBA, Depth, RGB16F, RG };
		enum class PixelType { UByte, Depth, Float };


		enum class TextureOption { WrapS, WrapT, WrapR, MinFilter, MagFilter };
		enum class TextureOptionValue { Repeat, ClampToEdge, Linear, LinearMipmap };

		// Cannot be used with target == Texture2D_Multisample, use SetTextureDataMultisampled instead
		static TextureID MakeTexture(TextureTarget target, PixelFormat gpuFormat, Vector2Int size, const void* data, PixelFormat dataFormat, PixelType dataType);
		// Can only be used width target == Texture2D_Multisample
		static TextureID MakeTextureMultisampled(TextureTarget target, PixelFormat gpuFormat, Vector2Int size, int sampleCount);
		
		// Cannot be used with target == Texture2D_Multisample, use SetTextureDataMultisampled instead
		static void SetTextureData(TextureID id, TextureTarget target, PixelFormat gpuFormat, Vector2Int size, const void* data, PixelFormat dataFormat, PixelType dataType);
		// Can only be used width target == Texture2D_Multisample
		static void SetTextureDataMultisampled(TextureID id, TextureTarget target, PixelFormat gpuFormat, Vector2Int size, int sampleCount);
		
		static void BindTexture(TextureID id, TextureTarget target);
		static void SetTextureOption(TextureID id, TextureTarget target, TextureOption option, TextureOptionValue value);
		static TextureOptionValue GetTextureOption(TextureID id, TextureTarget target, TextureOption option);
		static void DeleteTexture(TextureID id);

		static TextureID MakeCubemap();
		static void SetCubemapFace(TextureID id, CubemapFace face, PixelFormat gpuFormat, Vector2Int size, const void* data, PixelFormat dataFormat, PixelType dataType);

		static int GetActiveTexture();
		static void SetActiveTexture(int index);

		// Returns the number of valid indices for SetActiveTexture()
		static int GetTextureSlotCount();

		static void GenerateMipmaps(TextureTarget target);

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
		// sampleCount = -1 for no multisampling
		static BufferID MakeRenderBuffer(PixelType type, Vector2Int size, int sampleCount = -1);
		static void BindRenderBuffer(BufferID id);
		static void DeleteRenderBuffer(BufferID id);
		// sampleCount = -1 for no multisampling
		static void SetRenderBufferSize(BufferID id, PixelType type, Vector2Int size, int sampleCount = -1);
			
		// Frame buffers
		typedef unsigned int FrameBufferID;
		inline static constexpr TextureID InvalidFrameBufferID = 0;
		
		enum class FrameBufferTextureType { Color, Depth };

		static FrameBufferID MakeFrameBuffer();
		static void BindFrameBuffer(FrameBufferID id);
		static void BindFrameBufferRead(FrameBufferID id);
		static void BindFrameBufferDraw(FrameBufferID id);
		static void DeleteFrameBuffer(FrameBufferID id);
		static void BindFrameBufferTexture(FrameBufferID id, TextureID textureID, FrameBufferTextureType type);
		static void BindFrameBufferTextureMultisampled(FrameBufferID id, TextureID textureID, FrameBufferTextureType type);
		static void BindFrameBufferRenderBuffer(FrameBufferID id, BufferID renderBufferID, FrameBufferTextureType type);
		static void BindFrameBufferCubemapFace(FrameBufferID id, CubemapFace face, TextureID cubemap, FrameBufferTextureType type, int mip = 0);

		// Will blit from read framebuffer to draw framebuffer
		static void BlitFrameBuffer(Vector2Int inStart, Vector2Int inEnd, Vector2Int outStart, Vector2Int outEnd, FrameBufferTextureType type);

		static FrameBufferID GetBoundFrameBuffer();
		static FrameBufferID GetBoundDrawFrameBuffer();
		static FrameBufferID GetBoundReadFrameBuffer();
	};
}