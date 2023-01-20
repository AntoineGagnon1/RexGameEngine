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
	Shader::Shader(std::istream& data, RenderApi::CullingMode cullingMode, char priority, RenderApi::DepthFunction depth)
		: m_id(RenderApi::InvalidShaderID), m_cullingMode(cullingMode), m_priority(priority), m_depthFunction(depth)
	{
		// Parse the data to extract the shaders
		auto [vertexSource, fragmentSource, attributes] = ParseShaders(data);

		auto vertex = RenderApi::CompileShader(vertexSource, RenderApi::ShaderType::Vertex);
		auto fragment = RenderApi::CompileShader(fragmentSource, RenderApi::ShaderType::Fragment);

		if (vertex != RenderApi::InvalidShaderID && fragment != RenderApi::InvalidShaderID)
		{
			m_id = RenderApi::LinkShaders(vertex, fragment);

			RenderApi::DeleteShader(vertex); // Not needed anymore
			RenderApi::DeleteShader(fragment);

			// Cache the uniforms
			auto uniforms = RenderApi::GetShaderUniforms(m_id);
			for (auto& [name, uniformData] : uniforms)
			{
				decltype(Uniform::Attributes) attribs;
				if (attributes.contains(name))
					attribs = attributes[name];

				Uniform uniform{ std::get<0>(uniformData), std::get<1>(uniformData), attribs };
				m_uniforms[name] = uniform;
			}
		}
	}

	template<typename T> T& Unmove(T&& t) { return t; } // rvalue to lvalue cast

	Shader::Shader(const std::string& data, RenderApi::CullingMode cullingMode, char priority, RenderApi::DepthFunction depth)
		: Shader(Unmove(std::istringstream(data)), cullingMode, priority, depth)
	{

	}

	Shader::~Shader()
	{
		RenderApi::DeleteLinkedShader(m_id);
	}

	std::shared_ptr<Shader> Shader::FromFile(const std::string& path, RenderApi::CullingMode cullingMode, char priority, RenderApi::DepthFunction depth)
	{
		std::ifstream f(path);
		std::string str;
		if (f) 
		{
			std::stringstream ss;
			ss << f.rdbuf();
			return std::make_shared<Shader>(ss, cullingMode, priority, depth);
		}

		RE_LOG_ERROR("Could not open the shader at : {}", path);
		return std::shared_ptr<Shader>();
	}

	void Shader::Bind() const
	{
		RenderApi::SetCullingMode(m_cullingMode);
		RenderApi::SetDepthFunction(m_depthFunction);
		RenderApi::BindShader(m_id);
	}

	void Shader::UnBind()
	{
		RenderApi::SetCullingMode(RenderApi::CullingMode::Front);
		RenderApi::SetDepthFunction(RenderApi::DepthFunction::Less);
		RenderApi::BindShader(RenderApi::InvalidShaderID);
	}

	void Shader::SetUniformMatrix4(const std::string& name, const Matrix4& matrix)
	{
		RE_ASSERT(HasUniform(name), "No Matrix4 Uniform called {}", name);
		Bind();
		RenderApi::SetUniformMatrix4(m_uniforms[name].ID, matrix);
	}

	void Shader::SetUniformVector3(const std::string& name, const Vector3& vec)
	{
		RE_ASSERT(HasUniform(name), "No Vector3 Uniform called {}", name);
		Bind();
		RenderApi::SetUniformVector3(m_uniforms[name].ID, vec);
	}

	void Shader::SetUniformFloat(const std::string& name, float value)
	{
		RE_ASSERT(HasUniform(name), "No Float Uniform called {}", name);
		Bind();
		RenderApi::SetUniformFloat(m_uniforms[name].ID, value);
	}

	void Shader::SetUniformInt(const std::string& name, int value)
	{
		RE_ASSERT(HasUniform(name), "No Int Uniform called {}", name);
		Bind();
		RenderApi::SetUniformInt(m_uniforms[name].ID, value);
	}

	void Shader::RegisterParserUsing(const std::string& name, const std::string& replaceWith)
	{
		RE_ASSERT(!s_parserUsings.contains(name), "Shader parser #pragma using {} was already defined !", name)
		s_parserUsings.insert({name, replaceWith});
	}

	std::tuple<std::string, std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::any>>> Shader::ParseShaders(std::istream& fromStream)
	{
		std::ostringstream vertexStream;
		std::ostringstream fragmentStream;
		std::unordered_map<std::string, std::unordered_map<std::string, std::any>> attributes;

		std::string version = "#version 460 core"; // default version if not specified

		std::string line;
		std::ostringstream* writingTo = &vertexStream;

		// Parse #pragma directives
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
						std::string usingLine;
						std::istringstream usingStream(s_parserUsings[arguments[2]]);

						while (std::getline(usingStream, usingLine))
						{
							ParseLine(usingLine, attributes);
							(*writingTo) << usingLine << std::endl;
						}
					}
				}

				continue;
			}

			ParseLine(line, attributes);
			(*writingTo) << line << std::endl; // Save the line to the appropriate shader
		}

		// Append the version at the start of each shader
		std::string vertex = version + '\n' + vertexStream.str();
		std::string fragment = version + '\n' + fragmentStream.str();

		return std::make_tuple(vertex, fragment, attributes);
	}

	void Shader::ParseLine(std::string& line, std::unordered_map<std::string, std::unordered_map<std::string, std::any>>& attributes)
	{
		if (line.find("location") != std::string::npos) // vertex attribute location
		{
			ReplaceIfFound(line, "POSITION", std::to_string(PositionLocation));
			ReplaceIfFound(line, "NORMAL", std::to_string(NormalLocation));
			ReplaceIfFound(line, "TEXCOORDS", std::to_string(UVLocation));
		}

		if (auto pos = line.find("uniform"); pos != std::string::npos) // Attributes
		{
			std::smatch sm;

			std::string::const_iterator start = line.begin();
			std::string::const_iterator end = line.end();

			auto nameEnd = line.find(";");
			auto nameStart = line.substr(0, nameEnd).find_last_of(" ") + 1;
			std::string uniformName = line.substr(nameStart, nameEnd - nameStart);

			bool hadValidAttribute = false;

			while (std::regex_search(start, end, sm, s_attributeMatcher))
			{
				auto str = sm.str();
				auto argsStart = str.find("(");
				if (argsStart == std::string::npos)
				{
					auto name = str.substr(1, str.length() - 2);

					if (s_parserAttributes.contains(name))
					{
						attributes[uniformName][name] = s_parserAttributes[name]("");
						hadValidAttribute = true;
					}
				}
				else
				{
					auto name = str.substr(1, argsStart - 1);
					auto args = str.substr(argsStart + 1, str.length() - (argsStart + 3));

					if (s_parserAttributes.contains(name))
					{
						attributes[uniformName][name] = s_parserAttributes[name](args);
						hadValidAttribute = true;
					}
				}

				start = sm[0].second;
			}

			if(hadValidAttribute)
				line = line.substr(start - line.begin());
		}
	}
}
