#include "REPch.h"
#include "Material.h"

#include "Texture.h"
#include "TextureManager.h"
#include "Cubemap.h"

namespace RexEngine
{
	Material::Material(Asset<Shader> shader)
	{
		SetShader(shader);
	}

	void Material::Bind()
	{
		if (!m_shader)
		{
			RE_LOG_WARN("Trying to bind a material with no shader !");
			return;
		}

		m_shader->Bind();
		TextureManager::StartShader();
		// Set the uniforms
		for (auto& [name, value] : m_uniforms)
		{
			if (!m_shader->HasUniform(name) || m_shader->GetUniformType(name) != (RenderApi::UniformType)value.index())
				continue; // Some uniforms might be invalid if the shader changed, discard them

			using UT = RenderApi::UniformType;
			switch (value.index())
			{
			case (size_t)UT::Float:
				m_shader->SetUniformFloat(name, std::get<float>(value));
				break;
			case (size_t)UT::Vec3:
				m_shader->SetUniformVector3(name, std::get<Vector3>(value));
				break;
			case (size_t)UT::Int:
				m_shader->SetUniformInt(name, std::get<int>(value));
				break;
			case (size_t)UT::Mat4: 
				m_shader->SetUniformMatrix4(name, std::get<Matrix4>(value));
				break;
			case (size_t)UT::Sampler2D:  
				if (auto asset = std::get<Asset<Texture>>(value); asset)
				{
					int slot = TextureManager::GetTextureSlot(asset->GetId(), RenderApi::TextureTarget::Texture2D);
					m_shader->SetUniformInt(name, slot);
				}
				break;
			case (size_t)UT::SamplerCube:
				if (auto asset = std::get<Asset<Cubemap>>(value); asset)
				{
					int slot = TextureManager::GetTextureSlot(asset->GetId(), RenderApi::TextureTarget::Cubemap);
					m_shader->SetUniformInt(name, slot);
				}
				break;

			case (size_t)UT::Vec2: 
			case (size_t)UT::Vec4: 
			case (size_t)UT::Vec2I:
			case (size_t)UT::Vec3I:
			case (size_t)UT::Vec4I:
			case (size_t)UT::Double:
			case (size_t)UT::UInt: 
			case (size_t)UT::Bool: 
			case (size_t)UT::Mat3: 
			default:
				RE_ASSERT(false, "UniformType not supported");
			}
		}
	}

	void Material::SetShader(Asset<Shader> shader)
	{
		m_shader = shader;
		m_uniforms.clear();

		// Get all the uniforms
		if (shader && shader->IsValid())
		{
			auto names = shader->GetUniforms();

			for (auto& name : names)
			{
				auto type = shader->GetUniformType(name);

				using UT = RenderApi::UniformType;
				switch (type)
				{
				case UT::Float: m_uniforms[name] = (float)0.0f; break;
				case UT::Vec2:  m_uniforms[name] = Vector2(); break;
				case UT::Vec3:	m_uniforms[name] = Vector3(); break;
				case UT::Vec4:  m_uniforms[name] = Vector4(); break;
				case UT::Int:	m_uniforms[name] = (int)0; break;
				case UT::Vec2I: m_uniforms[name] = Vector2Int(); break;
				case UT::Vec3I: m_uniforms[name] = Vector3Int(); break;
				case UT::Vec4I: m_uniforms[name] = Vector4Int(); break;
				case UT::Double:m_uniforms[name] = (double)0.0; break;
				case UT::UInt:  m_uniforms[name] = (unsigned int)0; break;
				case UT::Bool:  m_uniforms[name] = (bool)false; break;
				case UT::Mat3:  m_uniforms[name] = Matrix3(); break;
				case UT::Mat4:  m_uniforms[name] = Matrix4(); break;
				case UT::Sampler2D:   m_uniforms[name] = Asset<Texture>(); break;
				case UT::SamplerCube: m_uniforms[name] = Asset<Cubemap>(); break;
				default:
					RE_ASSERT(false, "UniformType not supported");
				}
			}
		}
	}

	bool Material::IsValid() const
	{
		return (m_shader && m_shader->IsValid());
	}
}