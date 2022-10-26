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

	Shader::Shader(const std::string& data)
	{
		m_id = RenderApi::InvalidShaderID;

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

		if(m_id == RenderApi::InvalidShaderID)
			m_id = RenderApi::GetFallbackShader(); // Something went wrong
	}

	Shader::~Shader()
	{
		RenderApi::DeleteLinkedShader(m_id);
	}

	Shader Shader::FromFile(const std::string& path)
	{
		std::ifstream f(path);
		std::string str;
		if (f) 
		{
			std::ostringstream ss;
			ss << f.rdbuf();
			str = ss.str();
			return Shader(str);
		}

		RE_LOG_ERROR("Could not open the shader at : {}", path);
		return Shader(RenderApi::GetFallbackShader());
	}

	void Shader::Bind() const
	{
		RenderApi::BindShader(m_id);
	}

	void Shader::UnBind()
	{
		RenderApi::BindShader(RenderApi::InvalidShaderID);
	}


	std::tuple<std::string, std::string> Shader::ParseShaders(const std::string& data)
	{
		std::istringstream fromStream(data);
		std::ostringstream vertexStream;
		std::ostringstream fragmentStream;

		std::string version = "#version 330 core"; // default version if not specified

		std::string line;
		std::ostringstream* writingTo = &vertexStream;
		while (std::getline(fromStream, line)) 
		{	
			if (line.find("#pragma") != std::string::npos) // A directive
			{
				if (line.find("vertex") != std::string::npos) // Start of the vertex shader
					writingTo = &vertexStream;
				else if (line.find("fragment") != std::string::npos) // Start of the fragment shader
					writingTo = &fragmentStream;
				else if (line.find("version") != std::string::npos)
				{
					ReplaceIfFound(line, "#pragma ", "#"); // Convert #pragma version ... ... to #version ... ...
					version = line;
				}

				continue;
			}

			if (line.find("location") != std::string::npos) // vertex attribute location
			{
				ReplaceIfFound(line, "POSITION", std::to_string(PositionLocation));
				ReplaceIfFound(line, "NORMAL", std::to_string(NormalLocation));
			}

			(*writingTo) << line << std::endl; // Save the line to the appropriate shader
		}

		// Append the version at the start of each shader
		std::string vertex = version + '\n' + vertexStream.str();
		std::string fragment = version + '\n' + fragmentStream.str();

		return std::make_tuple(vertex, fragment);
	}
}
