#pragma once

#include <string>
#include <unordered_map>

#include "RenderApi.h"

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

	public:

		Shader(const std::string& data);
		Shader(RenderApi::ShaderID id) : m_id(id) {}
		~Shader();

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

		// Register a #pragma using clause for the shader parser
		static void RegisterParserUsing(const std::string& name, const std::string& replaceWith);
	private:

		static std::tuple<std::string, std::string> ParseShaders(const std::string& data);

	private:
		RenderApi::ShaderID m_id;

		std::unordered_map<std::string, int> m_uniforms;
		
		// the key is the name after #pragma using (key here) and the value is the text to add to the shader
		inline static std::unordered_map<std::string, std::string> s_parserUsings;
	};

}