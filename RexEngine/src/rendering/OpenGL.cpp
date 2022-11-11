#include <REPch.h>

#include "RenderApi.h"

#include "core/Libs.h"


namespace RexEngine::Internal {
	unsigned int BufferTypeToGLType(RenderApi::BufferType type)
	{
		switch (type)
		{
		case RenderApi::BufferType::Vertex:
			return GL_ARRAY_BUFFER;
		case RenderApi::BufferType::Indice:
			return GL_ELEMENT_ARRAY_BUFFER;
		case RenderApi::BufferType::Uniforms:
			return GL_UNIFORM_BUFFER;
		}

		return 0;
	}

	// <GL type, count, size(bytes)>
	std::tuple<unsigned int, int, size_t> AttributeTypeToGLType(RenderApi::VertexAttributeType type)
	{
		switch (type)
		{
		case RenderApi::VertexAttributeType::Float:
			return std::make_tuple(GL_FLOAT, 1, sizeof(float));
		case RenderApi::VertexAttributeType::Float2:
			return std::make_tuple(GL_FLOAT, 2, sizeof(float));
		case RenderApi::VertexAttributeType::Float3:
			return std::make_tuple(GL_FLOAT, 3, sizeof(float));
		case RenderApi::VertexAttributeType::Float4:
			return std::make_tuple(GL_FLOAT, 4, sizeof(float));
		}

		return std::make_tuple(0,0,0);
	}

	unsigned int TextureTargetToGL(RenderApi::TextureTarget target)
	{
		switch (target)
		{
		case RenderApi::TextureTarget::Texture2D:
			return GL_TEXTURE_2D;
		case RenderApi::TextureTarget::Cubemap:
			return GL_TEXTURE_CUBE_MAP;
		}

		RE_ASSERT(false, "Invalid texture target !");
		return 0;
	}

	unsigned int PixelFormatToGL(RenderApi::PixelFormat format)
	{
		switch (format)
		{
		case RenderApi::PixelFormat::RGB:
			return GL_RGB;
		case RenderApi::PixelFormat::RGBA:
			return GL_RGBA;
		case RenderApi::PixelFormat::Depth:
			return GL_DEPTH_COMPONENT;
		case RenderApi::PixelFormat::RGB16F:
			return GL_RGB16F;
		}

		RE_ASSERT(false, "Invalid texture format !");
		return 0;
	}

	unsigned int PixelTypeToGL(RenderApi::PixelType format)
	{
		switch (format)
		{
		case RenderApi::PixelType::UByte:
			return GL_UNSIGNED_BYTE;
		case RenderApi::PixelType::Float:
			return GL_FLOAT;
		case RenderApi::PixelType::Depth:
			return GL_DEPTH_COMPONENT24;
		}

		RE_ASSERT(false, "Invalid texture format !");
		return 0;
	}

	unsigned int TextureOptionToGL(RenderApi::TextureOption option)
	{
		switch (option)
		{
		case RenderApi::TextureOption::WrapS:
			return GL_TEXTURE_WRAP_S;
		case RenderApi::TextureOption::WrapT:
			return GL_TEXTURE_WRAP_T;
		case RenderApi::TextureOption::WrapR:
			return GL_TEXTURE_WRAP_R;
		case RenderApi::TextureOption::MinFilter:
			return GL_TEXTURE_MIN_FILTER;
		case RenderApi::TextureOption::MagFilter:
			return GL_TEXTURE_MAG_FILTER;
		}

		RE_ASSERT(false, "Invalid texture option !");
		return 0;
	}

	unsigned int TextureOptionValueToGL(RenderApi::TextureOptionValue value)
	{
		switch (value)
		{
		case RenderApi::TextureOptionValue::Repeat:
			return GL_REPEAT;
		case RenderApi::TextureOptionValue::ClampToEdge:
			return GL_CLAMP_TO_EDGE;
		case RenderApi::TextureOptionValue::Linear:
			return GL_LINEAR;
		}

		RE_ASSERT(false, "Invalid texture option value !");
		return 0;
	}

	unsigned int DepthFunctionToGL(RenderApi::DepthFunction function)
	{
		switch (function)
		{
		case RenderApi::DepthFunction::Less:
			return GL_LESS;
		case RenderApi::DepthFunction::LessEqual:
			return GL_LEQUAL;
		case RenderApi::DepthFunction::Greater:
			return GL_GREATER;
		case RenderApi::DepthFunction::GreaterEqual:
			return GL_GEQUAL;
		}

		RE_ASSERT(false, "Invalid depth function !");
		return 0;
	}

	void GlCheckErrors()
	{
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			RE_LOG_ERROR("OpenGL error : {}", err);
			RE_DEBUG_BREAK();
		}
	}
}

#define GL_CALL(x) x;Internal::GlCheckErrors();

namespace RexEngine
{
	void RenderApi::Init()
	{
		GL_CALL(glEnable(GL_DEPTH_TEST));
		GL_CALL(glClearDepthf(1.0f));

		GL_CALL(glFrontFace(GL_CW));
	}



	RenderApi::ShaderID RenderApi::CompileShader(const std::string& source, ShaderType type)
	{
		static constexpr unsigned int TypeToGLType[]{ GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
		static const std::string TypeToName[]{ "Vertex", "Fragment" };

		// Create the shader
		ShaderID id = GL_CALL(glCreateShader(TypeToGLType[(int)type]));

		const char* sourcePtr = source.c_str();
		GL_CALL(glShaderSource(id, 1, &sourcePtr, NULL));
		GL_CALL(glCompileShader(id));

		// Check for errors
		int success;
		GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &success));
		if (success == GL_FALSE)
		{
			char infoLog[512];
			GL_CALL(glGetShaderInfoLog(id, 512, NULL, infoLog));
			RE_LOG_ERROR("{} shader compilation failed : {}", TypeToName[(int)type], infoLog);
			return InvalidShaderID;
		}

		return id;
	}

	RenderApi::ShaderID RenderApi::LinkShaders(ShaderID vertex, ShaderID fragment)
	{
		ShaderID id = GL_CALL(glCreateProgram());
		GL_CALL(glAttachShader(id, vertex));
		GL_CALL(glAttachShader(id, fragment));

		GL_CALL(glLinkProgram(id));

		// Check for errors
		int success;
		GL_CALL(glGetProgramiv(id, GL_LINK_STATUS, &success));
		if (success == GL_FALSE)
		{
			char infoLog[512];
			GL_CALL(glGetProgramInfoLog(id, 512, NULL, infoLog));
			RE_LOG_ERROR("Shader linking failed : {}", infoLog);
			return InvalidShaderID;
		}

		return id;
	}

	void RenderApi::DeleteShader(ShaderID id)
	{
		GL_CALL(glDeleteShader(id));
	}
	
	void RenderApi::DeleteLinkedShader(ShaderID id)
	{
		GL_CALL(glDeleteProgram(id));
	}

	void RenderApi::BindShader(ShaderID id)
	{
		GL_CALL(glUseProgram(id));
	}

	std::unordered_map<std::string, int> RenderApi::GetShaderUniforms(ShaderID id)
	{
		std::unordered_map<std::string, int>  uniforms;

		GLint count;
		GLint size; // size of the variable
		GLenum type; // type of the variable (GL_FLOAT, GL_FLOAT_VEC2, ...)

		const GLsizei bufSize = 32;
		GLchar name[bufSize];

		GL_CALL(glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count));

		for (GLuint i = 0; i < count; i++)
		{
			GL_CALL(glGetActiveUniform(id, i, bufSize, NULL, &size, &type, name));
			int index = GL_CALL(glGetUniformLocation(id, name));
			uniforms.insert({ name, index });
		}

		return uniforms;
	}

	void RenderApi::SetUniformMatrix4(int location, const Matrix4& matrix)
	{
		GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
	}

	void RenderApi::SetUniformVector3(int location, const Vector3& vec)
	{
		GL_CALL(glUniform3fv(location, 1, &vec[0]));
	}

	void RenderApi::SetUniformFloat(int location, float value)
	{
		GL_CALL(glUniform1f(location, value));
	}

	void RenderApi::SetUniformInt(int location, int value)
	{
		GL_CALL(glUniform1i(location, value));
	}


	RenderApi::BufferID RenderApi::MakeBuffer()
	{
		BufferID id;
		GL_CALL(glGenBuffers(1, &id));
		return id;
	}

	void RenderApi::BindBuffer(BufferID id, BufferType type)
	{
		GL_CALL(glBindBuffer(Internal::BufferTypeToGLType(type), id));
	}

	void RenderApi::DeleteBuffer(BufferID id)
	{
		GL_CALL(glDeleteBuffers(1, &id));
	}

	void RenderApi::SetBufferData(BufferID id, BufferType type, BufferMode mode, const uint8_t* data, size_t length)
	{
		BindBuffer(id, type);
		GL_CALL(glBufferData(Internal::BufferTypeToGLType(type), length, data, mode == BufferMode::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
	}

	void RenderApi::SubBufferData(BufferID id, BufferType type, size_t offset, size_t size, const void* data)
	{
		BindBuffer(id, type);
		GL_CALL(glBufferSubData(Internal::BufferTypeToGLType(type), offset, size, data));
	}

	void RenderApi::BindBufferBase(BufferID id, BufferType type, int location)
	{
		GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, location, id));
	}



	RenderApi::VertexAttribID RenderApi::MakeVertexAttributes(std::span<std::tuple<VertexAttributeType, int>> attributes, BufferID vertexBuffer, BufferID indices)
	{
		// Create the vao
		unsigned int vao;
		GL_CALL(glGenVertexArrays(1, &vao));

		BindVertexAttributes(vao);

		// Bind the data
		BindBuffer(vertexBuffer, RenderApi::BufferType::Vertex);
		BindBuffer(indices, RenderApi::BufferType::Indice);
		
		// Calculate the stride
		int stride = 0;
		for (auto&& type : attributes)
		{
			auto [_, count, size] = Internal::AttributeTypeToGLType(std::get<0>(type));
			stride += count * size;
		}

		// Set the attributes
		size_t offset = 0; // Current offset
		for (int i = 0; i < attributes.size(); i++)
		{
			auto [type, count, size] = Internal::AttributeTypeToGLType(std::get<0>(attributes[i]));
			int location = std::get<1>(attributes[i]);
			GL_CALL(glEnableVertexAttribArray(location));
			GL_CALL(glVertexAttribPointer(location, count, type, GL_FALSE, stride, (void*)offset));
			
			offset += size * count;
		}

		BindVertexAttributes(0);
		return vao;
	}

	void RenderApi::DeleteVertexAttributes(VertexAttribID id)
	{
		GL_CALL(glDeleteVertexArrays(1, &id));
	}

	void RenderApi::BindVertexAttributes(VertexAttribID id)
	{
		GL_CALL(glBindVertexArray(id));
	}

	RenderApi::TextureID RenderApi::MakeTexture(TextureTarget target, PixelFormat gpuFormat, Vector2Int size, const void* data, PixelFormat dataFormat, PixelType dataType)
	{
		TextureID id;
		GL_CALL(glGenTextures(1, &id));
		BindTexture(id, target);

		GL_CALL(glTexImage2D(
			Internal::TextureTargetToGL(target), 0,
			Internal::PixelFormatToGL(gpuFormat),
			size.x, size.y, 0,
			Internal::PixelFormatToGL(dataFormat),
			Internal::PixelTypeToGL(dataType),
			data
		));

		return id;
	}

	void RenderApi::BindTexture(TextureID id, TextureTarget target)
	{
		GL_CALL(glBindTexture(Internal::TextureTargetToGL(target), id));
	}

	void RenderApi::SetTextureOption(TextureID id, TextureTarget target, TextureOption option, TextureOptionValue value)
	{
		BindTexture(id, target);
		GL_CALL(glTexParameteri(Internal::TextureTargetToGL(target), Internal::TextureOptionToGL(option), Internal::TextureOptionValueToGL(value)));
	}

	void RenderApi::DeleteTexture(TextureID id)
	{
		GL_CALL(glDeleteTextures(1, &id));
	}

	RenderApi::TextureID RenderApi::MakeCubemap()
	{
		TextureID id;
		GL_CALL(glGenTextures(1, &id));
		return id;
	}

	void RenderApi::SetCubemapFace(TextureID id, CubemapFace face, PixelFormat gpuFormat, Vector2Int size, const void* data, PixelFormat dataFormat, PixelType dataType)
	{
		BindTexture(id, TextureTarget::Cubemap);

		GL_CALL(glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)face, 0,
			Internal::PixelFormatToGL(gpuFormat),
			size.x, size.y, 0,
			Internal::PixelFormatToGL(dataFormat),
			Internal::PixelTypeToGL(dataType),
			data
		));
	}

	int RenderApi::GetActiveTexture()
	{
		GLint active;
		GL_CALL(glGetIntegerv(GL_ACTIVE_TEXTURE, &active));
		return active;
	}

	void RenderApi::SetActiveTexture(int index)
	{
		GL_CALL(glActiveTexture(GL_TEXTURE0 + index));
	}



	void RenderApi::SetViewportSize(Vector2Int size)
	{
		GL_CALL(glViewport(0, 0, size.x, size.y));
	}

	Vector2Int RenderApi::GetViewportSize()
	{
		int viewport[4];
		GL_CALL(glGetIntegerv(GL_VIEWPORT, viewport));

		return Vector2Int(viewport[2], viewport[3]);
	}

	void RenderApi::ClearColorBit()
	{
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
	}

	void RenderApi::ClearDepthBit()
	{
		GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
	}

	void RenderApi::DrawElements(size_t count)
	{
		GL_CALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0));
	}


	void RenderApi::SetCullingMode(CullingMode mode)
	{
		switch (mode)
		{
		case RexEngine::RenderApi::CullingMode::Front:
			GL_CALL(glEnable(GL_CULL_FACE));
			GL_CALL(glCullFace(GL_BACK)); // Inverted because we want to keep the front face
			break;
		case RexEngine::RenderApi::CullingMode::Back:
			GL_CALL(glEnable(GL_CULL_FACE));
			GL_CALL(glCullFace(GL_FRONT));
			break;
		case RexEngine::RenderApi::CullingMode::Both:
			GL_CALL(glDisable(GL_CULL_FACE));
			break;
		}
		
	}


	void RenderApi::SetDepthFunction(DepthFunction function)
	{
		GL_CALL(glDepthFunc(Internal::DepthFunctionToGL(function)));
	}

	RenderApi::BufferID RenderApi::MakeRenderBuffer(PixelType type, Vector2Int size)
	{
		BufferID id;
		GL_CALL(glGenRenderbuffers(1, &id));

		GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, id));
		GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, Internal::PixelTypeToGL(type), size.x, size.y));
		return id;
	}

	void RenderApi::BindRenderBuffer(BufferID id)
	{
		GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, id));
	}

	void RenderApi::DeleteRenderBuffer(BufferID id)
	{
		GL_CALL(glDeleteRenderbuffers(1, &id));
	}

	RenderApi::FrameBufferID RenderApi::MakeFrameBuffer()
	{
		FrameBufferID id;
		GL_CALL(glGenFramebuffers(1, &id));
		return id;
	}

	void RenderApi::BindFrameBuffer(FrameBufferID id)
	{
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, id));
	}

	void RenderApi::DeleteFrameBuffer(FrameBufferID id)
	{
		GL_CALL(glDeleteFramebuffers(1, &id));
	}

	void RenderApi::BindFrameBufferTexture(FrameBufferID id, TextureID textureID, FrameBufferTextureType type)
	{
		BindFrameBuffer(id);

		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, 
			type == FrameBufferTextureType::Color ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, textureID, 0));
	}

	void RenderApi::BindFrameBufferRenderBuffer(FrameBufferID id, BufferID renderBufferID, FrameBufferTextureType type)
	{
		BindFrameBuffer(id);
		
		//BindRenderBuffer(renderBufferID);

		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
			type == FrameBufferTextureType::Color ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_ATTACHMENT
			, GL_RENDERBUFFER, renderBufferID));
	}

	void RenderApi::BindFrameBufferCubemapFace(FrameBufferID id, CubemapFace face, TextureID cubemap, FrameBufferTextureType type)
	{
		BindFrameBuffer(id);

		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, 
			type == FrameBufferTextureType::Color ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)face, cubemap, 0));
	}
}