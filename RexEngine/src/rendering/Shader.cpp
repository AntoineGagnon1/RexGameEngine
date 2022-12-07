#include "REPch.h"
#include "Shader.h"

namespace {
	void ReplaceIfFound(std::string& str, const std::string& find, const std::string& replace)
	{
		auto pos = str.find(find);
		if (pos != std::string::npos)
			str.replace(pos, find.size(), replace);
	}
}

namespace RexEngine
{
	Shader::Shader(std::istream& data)
		: m_id(RenderApi::InvalidShaderID)
	{
		// Parse the data to extract the shaders
		auto [vertexSource, fragmentSource] = ParseShaders(data);

		auto vertex = RenderApi::CompileShader(vertexSource, RenderApi::ShaderType::Vertex);
		auto fragment = RenderApi::CompileShader(fragmentSource, RenderApi::ShaderType::Fragment);

		if (vertex != RenderApi::InvalidShaderID && fragment != RenderApi::InvalidShaderID)
		{
			m_id = RenderApi::LinkShaders(vertex, fragment);

			RenderApi::DeleteShader(vertex); // Not needed anymore
			RenderApi::DeleteShader(fragment);
		}

		// Cache the uniforms
		m_uniforms = RenderApi::GetShaderUniforms(m_id);
	}

	Shader::Shader(const std::string& data)
		: Shader((std::istream&)std::istringstream(data))
	{

	}

	Shader::~Shader()
	{
		RenderApi::DeleteLinkedShader(m_id);
	}

	std::shared_ptr<Shader> Shader::FromFile(const std::string& path)
	{
		std::ifstream f(path);
		std::string str;
		if (f) 
		{
			std::stringstream ss;
			ss << f.rdbuf();
			return std::make_shared<Shader>(ss);
		}

		RE_LOG_ERROR("Could not open the shader at : {}", path);
		return std::shared_ptr<Shader>();
	}

	void Shader::Bind() const
	{
		RenderApi::BindShader(m_id);
	}

	void Shader::UnBind()
	{
		RenderApi::BindShader(RenderApi::InvalidShaderID);
	}

	void Shader::SetUniformMatrix4(const std::string& name, const Matrix4& matrix)
	{
		RE_ASSERT(HasUniform(name), "No Matrix4 Uniform called {}", name);
		Bind();
		RenderApi::SetUniformMatrix4(m_uniforms[name], matrix);
	}

	void Shader::SetUniformVector3(const std::string& name, const Vector3& vec)
	{
		RE_ASSERT(HasUniform(name), "No Vector3 Uniform called {}", name);
		Bind();
		RenderApi::SetUniformVector3(m_uniforms[name], vec);
	}

	void Shader::SetUniformFloat(const std::string& name, float value)
	{
		RE_ASSERT(HasUniform(name), "No Float Uniform called {}", name);
		Bind();
		RenderApi::SetUniformFloat(m_uniforms[name], value);
	}

	void Shader::SetUniformInt(const std::string& name, int value)
	{
		RE_ASSERT(HasUniform(name), "No Int Uniform called {}", name);
		Bind();
		RenderApi::SetUniformInt(m_uniforms[name], value);
	}

	void Shader::RegisterParserUsing(const std::string& name, const std::string& replaceWith)
	{
		RE_ASSERT(!s_parserUsings.contains(name), "Shader parser #pragma using {} was already defined !", name)
		s_parserUsings.insert({name, replaceWith});
	}

	std::tuple<std::string, std::string> Shader::ParseShaders(std::istream& fromStream)
	{
		std::ostringstream vertexStream;
		std::ostringstream fragmentStream;

		std::string version = "#version 420 core"; // default version if not specified

		std::string line;
		std::ostringstream* writingTo = &vertexStream;
		while (std::getline(fromStream, line))
		{	
			
			if (line.starts_with("#pragma")) // A directive
			{
				auto arguments = StringHelper::Split(line, ' ');
				if (arguments.size() <= 1)
					continue; // No arguments

				if (arguments[1] == "vertex") // Start of the vertex shader
					writingTo = &vertexStream;
				else if (arguments[1] == "fragment") // Start of the fragment shader
					writingTo = &fragmentStream;
				else if (arguments[1] == "version")
				{
					ReplaceIfFound(line, "#pragma ", "#"); // Convert #pragma version ... ... to #version ... ...
					version = line;
				}
				else if (arguments[1] == "using" && arguments.size() >= 3)
				{
					if(!s_parserUsings.contains(arguments[2]))
					{ 
						RE_LOG_ERROR("Shader parser error : #pragma using {}", arguments[2]);
					}
					else
					{
						(*writingTo) << s_parserUsings[arguments[2]] << std::endl;
					}
				}

				continue;
			}

			if (line.find("location") != std::string::npos) // vertex attribute location
			{
				ReplaceIfFound(line, "POSITION", std::to_string(PositionLocation));
				ReplaceIfFound(line, "NORMAL", std::to_string(NormalLocation));
				ReplaceIfFound(line, "TEXCOORDS", std::to_string(UVLocation));
			}

			(*writingTo) << line << std::endl; // Save the line to the appropriate shader
		}

		// Append the version at the start of each shader
		std::string vertex = version + '\n' + vertexStream.str();
		std::string fragment = version + '\n' + fragmentStream.str();

		return std::make_tuple(vertex, fragment);
	}
}
