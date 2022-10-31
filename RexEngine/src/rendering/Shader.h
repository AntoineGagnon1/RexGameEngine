#pragma once

#include <string>
#include <unordered_map>

#include "RenderApi.h"

namespace RexEngine
{

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

		static Shader FromFile(const std::string& path);

		auto GetID() const { return m_id; }
		bool IsValid() const { return m_id != RenderApi::InvalidShaderID; };

		void Bind() const;
		static void UnBind();

		bool HasUniform(const std::string& name) { return m_uniforms.contains(name); }

		void SetUniformMatrix4(const std::string& name, const Matrix4& matrix);

	private:

		static std::tuple<std::string, std::string> ParseShaders(const std::string& data);

	private:
		RenderApi::ShaderID m_id;

		std::unordered_map<std::string, int> m_uniforms;
	};

}