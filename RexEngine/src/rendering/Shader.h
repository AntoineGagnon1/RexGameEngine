#pragma once

#include <string>
#include <unordered_map>
#include <ranges>
#include <any>
#include <regex>

#include "RenderApi.h"
//#include "../core/Serialization.h"

namespace RexEngine
{
	// Uniform blocks
	// 1 : Scene data (forwardRenderer)
	// 2 : Model data (renderQueue)
	// 3 : Lights (Point and Directional)
	// 4 : Spot Lights

	template<typename T>
	concept UniformAttribute = requires(const std::string& args)
	{
		T(args);
	};

	struct Uniform
	{
		int ID = -1; // RenderApi id in the shader
		RenderApi::UniformType Type = RenderApi::UniformType::Float;

		std::unordered_map<std::string, std::any> Attributes;
	};

	class Shader
	{
	public:
		// Location of these vertex attributes
		inline static constexpr int PositionLocation = 0;
		inline static constexpr int NormalLocation = 1;
		inline static constexpr int UVLocation = 2;

	public:
		Shader(std::istream& data, RenderApi::CullingMode cullingMode = RenderApi::CullingMode::Front, char priority = 0, RenderApi::DepthFunction depth = RenderApi::DepthFunction::Less);
		Shader(const std::string& data, RenderApi::CullingMode cullingMode = RenderApi::CullingMode::Front, char priority = 0, RenderApi::DepthFunction depth = RenderApi::DepthFunction::Less);
		~Shader();

		Shader(const Shader&) = delete;


		static std::shared_ptr<Shader> FromFile(const std::string& path, RenderApi::CullingMode cullingMode = RenderApi::CullingMode::Front, char priority = 0, RenderApi::DepthFunction depth = RenderApi::DepthFunction::Less);
		auto GetID() const { return m_id; }
		bool IsValid() const { return m_id != RenderApi::InvalidShaderID; };

		auto& CullingMode() { return m_cullingMode; }
		auto& Priority() { return m_priority; }
		auto& DepthFunction() { return m_depthFunction; }

		auto CullingMode() const { return m_cullingMode; }
		auto Priority() const { return m_priority; }
		auto DepthFunction() const { return m_depthFunction; }

		// Will also set the culling mode
		void Bind() const;
		// Will also set the culling mode to Front
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
				return m_uniforms[name].Type;
			RE_LOG_ERROR("No uniform named : {}", name);
			return (RenderApi::UniformType)-1;
		}

		auto GetUniformAttributes(const std::string& name) 
		{
			if (m_uniforms.contains(name))
				return m_uniforms[name].Attributes;
			RE_LOG_ERROR("No uniform named : {}", name);
			return decltype(Uniform::Attributes)();
		}

		// Register a #pragma using clause for the shader parser
		static void RegisterParserUsing(const std::string& name, const std::string& replaceWith);

		// Attributes need a constructor like : T(const std::string& args)
		template<UniformAttribute T>
		inline static void RegisterAttribute(const std::string& name)
		{
			s_parserAttributes.insert({ name, [](auto args) { return T(args); } });
		}

		template<typename Archive>
		inline static std::shared_ptr<Shader> LoadFromAssetFile([[maybe_unused]]Guid _, Archive& metaDataArchive, std::istream& assetFile)
		{
			int cullingMode, depthFunction;
			char priority;
			metaDataArchive(CUSTOM_NAME(cullingMode, "CullingMode"),
				CUSTOM_NAME(priority, "Priority"),
				CUSTOM_NAME(depthFunction, "DepthFunction"));
			return std::make_shared<Shader>(assetFile, (RenderApi::CullingMode)cullingMode, priority, (RenderApi::DepthFunction)depthFunction);
		}

		template<typename Archive>
		void SaveToAssetFile(Archive& metaDataArchive)
		{
			metaDataArchive(CUSTOM_NAME((int)m_cullingMode, "CullingMode"),
				CUSTOM_NAME(m_priority, "Priority"),
				CUSTOM_NAME((int)m_depthFunction, "DepthFunction"));
		}

		template<typename Archive>
		inline static void CreateMetaData(Archive& metaDataArchive)
		{
			metaDataArchive(CUSTOM_NAME((int)RenderApi::CullingMode::Front, "CullingMode"),
				CUSTOM_NAME((char)0, "Priority"),
				CUSTOM_NAME((int)RenderApi::DepthFunction::Less, "DepthFunction"));
		}

	private:

		static std::tuple<std::string, std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::any>>> ParseShaders(std::istream& data);

		//															uniform name, attribute
		static void ParseLine(std::string& line, std::unordered_map<std::string, std::unordered_map<std::string, std::any>>& attributes);

	private:
		RenderApi::ShaderID m_id;

		RenderApi::CullingMode m_cullingMode;
		char m_priority;
		RenderApi::DepthFunction m_depthFunction;

		std::unordered_map<std::string, Uniform> m_uniforms;
		
		// the key is the name after #pragma using (key here) and the value is the text to add to the shader
		inline static std::unordered_map<std::string, std::string> s_parserUsings;

		inline static std::unordered_map<std::string, std::function<std::any(const std::string&)>> s_parserAttributes;

		inline static const std::regex s_attributeMatcher = std::regex(R"(\[.*?\])");
	};

}