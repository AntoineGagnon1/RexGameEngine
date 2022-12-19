#pragma once

#include <string>
#include <unordered_map>
#include <ranges>

#include "RenderApi.h"
//#include "../core/Serialization.h"

namespace RexEngine
{
	// Uniform blocks
	// 1 : Scene data (forwardRenderer)
	// 2 : Model data (renderQueue)
	// 3 : Lighting

	class Shader
	{
	public:
		// Location of these vertex attributes
		inline static constexpr int PositionLocation = 0;
		inline static constexpr int NormalLocation = 1;
		inline static constexpr int UVLocation = 2;

	public:
		Shader(std::istream& data);
		Shader(const std::string& data);
		~Shader();

		Shader(const Shader&) = delete;


		static std::shared_ptr<Shader> FromFile(const std::string& path);
		auto GetID() const { return m_id; }
		bool IsValid() const { return m_id != RenderApi::InvalidShaderID; };

		void Bind() const;
		static void UnBind();

		bool HasUniform(const std::string& name) { return m_uniforms.contains(name); }

		void SetUniformMatrix4(const std::string& name, const Matrix4& matrix);
		void SetUniformVector3(const std::string& name, const Vector3& vec);
		void SetUniformFloat(const std::string& name, float value);
		void SetUniformInt(const std::string& name, int value);

		std::vector<std::string> GetUniforms()
		{
			auto keys = std::views::keys(m_uniforms);
			return std::vector<std::string>{ keys.begin(), keys.end() };
		}

		// Will return -1 if the uniform does not exists
		RenderApi::UniformType GetUniformType(const std::string& name)
		{
			if (m_uniforms.contains(name))
				return std::get<1>(m_uniforms[name]);
			RE_LOG_ERROR("No uniform named : {}", name);
			return (RenderApi::UniformType)-1;
		}

		// Register a #pragma using clause for the shader parser
		static void RegisterParserUsing(const std::string& name, const std::string& replaceWith);

		template<typename Archive>
		inline static std::shared_ptr<Shader> LoadFromAssetFile(Guid _, Archive& metaDataArchive, std::istream& assetFile)
		{
			return std::make_shared<Shader>(assetFile);
		}

	private:

		static std::tuple<std::string, std::string> ParseShaders(std::istream& data);

	private:
		RenderApi::ShaderID m_id;

		std::unordered_map<std::string, std::tuple<int, RenderApi::UniformType>> m_uniforms;
		
		// the key is the name after #pragma using (key here) and the value is the text to add to the shader
		inline static std::unordered_map<std::string, std::string> s_parserUsings;
	};

}