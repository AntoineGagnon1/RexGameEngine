#include <REPch.h>

#include "RenderApi.h"

#include "core/Libs.h"

namespace {
	unsigned int BufferTypeToGLType(RexEngine::RenderApi::BufferType type)
	{
		switch (type)
		{
		case RexEngine::RenderApi::BufferType::Vertex:
			return GL_ARRAY_BUFFER;
		case RexEngine::RenderApi::BufferType::Indice:
			return GL_ELEMENT_ARRAY_BUFFER;
		}

		return 0;
	}

	// <GL type, count, size(bytes)>
	std::tuple<unsigned int, int, size_t> AttributeTypeToGLType(RexEngine::RenderApi::VertexAttributeType type)
	{
		switch (type)
		{
		case RexEngine::RenderApi::VertexAttributeType::Float:
			return std::make_tuple(GL_FLOAT, 1, sizeof(float));
		case RexEngine::RenderApi::VertexAttributeType::Float2:
			return std::make_tuple(GL_FLOAT, 2, sizeof(float));
		case RexEngine::RenderApi::VertexAttributeType::Float3:
			return std::make_tuple(GL_FLOAT, 3, sizeof(float));
		case RexEngine::RenderApi::VertexAttributeType::Float4:
			return std::make_tuple(GL_FLOAT, 4, sizeof(float));
		}

		return std::make_tuple(0,0,0);
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

#define GL_CALL(x) x;GlCheckErrors();

namespace RexEngine
{
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

	RenderApi::ShaderID RenderApi::GetFallbackShader()
	{
		static const ShaderID FallbackShader = LinkShaders(
			CompileShader("#version 330 core\nlayout (location = 0) in vec3 aPos;\nvoid main()\n{\ngl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n}", ShaderType::Vertex),
			CompileShader("#version 330 core\nout vec4 FragColor;\nvoid main()\n{\nFragColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);\n}", ShaderType::Fragment)
		);

		return FallbackShader;
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
			uniforms.insert({ name, i });
		}

		return uniforms;
	}

	void RenderApi::SetUniformMatrix4(int location, const Matrix4& matrix)
	{
		GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
	}



	RenderApi::BufferID RenderApi::MakeBuffer()
	{
		BufferID id;
		GL_CALL(glGenBuffers(1, &id));
		return id;
	}

	void RenderApi::BindBuffer(BufferID id, BufferType type)
	{
		GL_CALL(glBindBuffer(BufferTypeToGLType(type), id));
	}

	void RenderApi::DeleteBuffer(BufferID id)
	{
		GL_CALL(glDeleteBuffers(1, &id));
	}

	void RenderApi::SetBufferData(BufferID id, BufferType type, BufferMode mode, const uint8_t* data, size_t length)
	{
		BindBuffer(id, type);
		GL_CALL(glBufferData(BufferTypeToGLType(type), length, data, mode == BufferMode::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
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
			auto [_, count, size] = AttributeTypeToGLType(std::get<0>(type));
			stride += count * size;
		}

		// Set the attributes
		size_t offset = 0; // Current offset
		for (int i = 0; i < attributes.size(); i++)
		{
			auto [type, count, size] = AttributeTypeToGLType(std::get<0>(attributes[i]));
			int location = std::get<1>(attributes[i]);
			GL_CALL(glVertexAttribPointer(location, count, type, GL_FALSE, stride, (void*)offset));
			GL_CALL(glEnableVertexAttribArray(location));
			
			offset += size * count;
		}

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

	void RenderApi::DrawElements(size_t count)
	{
		GL_CALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0));
	}
}