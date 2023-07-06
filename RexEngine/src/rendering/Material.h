#pragma once
#include <memory>
#include <variant>
#include <type_traits>

#include "../core/Guid.h"
#include "../core/Log.h"
#include "../assets/AssetManager.h"
#include "Texture.h"
#include "Cubemap.h"
#include "Shader.h"

namespace RexEngine
{
	class Material
	{
	public:
		using UniformType = std::variant<float, Vector2, Vector3, Vector4,
										 int, Vector2Int, Vector3Int, Vector4Int,
										 double, unsigned int, bool,
										 Matrix3, Matrix4,
										 Asset<Texture>, Asset<Cubemap>>;

	public:

		Material(Asset<Shader> shader);
		
		// Use Asset<Texture> and Asset<Cubemap> for sampler2D and samplerCube
		template<typename T>
		T& GetUniform(const std::string& name)
		{
			return std::get<T>(m_uniforms[name]);
		}

		UniformType& GetUniform(const std::string& name)
		{
			return m_uniforms[name];
		}

		auto GetUniformAttributes(const std::string& name) 
		{
			if(m_shader)
				return m_shader->GetUniformAttributes(name);
			return  decltype(std::declval<Shader>().GetUniformAttributes(""))();
		}

		// Get the name of all the uniforms
		std::vector<std::string> GetUniforms() const
		{
			auto keys = std::views::keys(m_uniforms);
			return std::vector<std::string>{ keys.begin(), keys.end() };
		}

		void SetShader(Asset<Shader> shader);

		Asset<Shader> GetShader() const { return m_shader; }

		// Bind the shader and set the uniforms
		void Bind();

		static void UnBind()
		{
			Shader::UnBind();
		}

		bool IsValid() const;

		template<typename Archive>
		inline static std::shared_ptr<Material> LoadFromAssetFile([[maybe_unused]]Guid _, [[maybe_unused]]Archive& metaDataArchive, std::istream& assetFile)
		{
			JsonDeserializer archive(assetFile);
			Guid guid;
			archive(CUSTOM_NAME(guid, "Shader"));

			auto ptr = std::make_shared<Material>(AssetManager::GetAsset<Shader>(guid));
			if(ptr)
				archive(CUSTOM_NAME(ptr->m_uniforms, "Uniforms"));

			return ptr;
		}

		template<typename Archive>
		void SaveToAssetFile([[maybe_unused]] Archive& metaDataArchive, std::ostream& assetFile)
		{
			JsonSerializer archive(assetFile);
			archive(CUSTOM_NAME(m_shader.GetAssetGuid(), "Shader"));

			// Sort the uniforms by name
			std::map<std::string, UniformType> uniforms;
			for (auto& pair : m_uniforms)
				uniforms.insert(pair);

			archive(CUSTOM_NAME(uniforms, "Uniforms"));
		}

	private:
		
		Asset<Shader> m_shader;

		// IMPORTANT : the indices of the variant match RenderApi::UniformType
		std::unordered_map < std::string, UniformType> m_uniforms;
	};
}