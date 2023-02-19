#pragma once

#include <memory>
#include <ranges>

#include "../rendering/Material.h"
#include "../rendering/Mesh.h"
#include "../rendering/Cubemap.h"
#include "../math/Vectors.h"
#include "../math/Quaternion.h"
#include "../math/Matrix.h"
#include "../core/Color.h"
#include "../core/Serialization.h"
#include "../assets/AssetManager.h"

#include "Entity.h"
#include "Scene.h"
#include "ComponentFactory.h"

namespace RexEngine
{
	struct TagComponent
	{
		std::string name;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(name));
		}
	};
	RE_REGISTER_COMPONENT(TagComponent, "Tag")

	struct MeshRendererComponent
	{
		Asset<Material> material;
		Asset<Mesh> mesh;

		template<typename Archive>
		void serialize(Archive& archive) 
		{
			archive(KEEP_NAME(material), KEEP_NAME(mesh));
		}
	};
	RE_REGISTER_COMPONENT(MeshRendererComponent, "MeshRenderer")

	struct TransformComponent
	{
		Vector3 position = Vector3(0,0,0);
		Vector3 scale = Vector3(1,1,1);
		Quaternion rotation = Quaternion(0,0,0,1);

		Entity parent;

		// Local
		Vector3 Forward() const { return rotation.RotateVector(Directions::Forward); }
		// Local
		Vector3 Up() const { return rotation.RotateVector(Directions::Up); }
		// Local
		Vector3 Right() const { return rotation.RotateVector(Directions::Right); }

		Vector3 GlobalForward() const { return GlobalRotation() * Directions::Forward; }

		Matrix4 GetTransform() const
		{
			return Matrix4::MakeTransform(position, rotation, scale);
		}

		Matrix4 GetGlobalTransform() const
		{
			Matrix4 parentMatrix = Matrix4::Identity;
			if (parent && parent.HasComponent<TransformComponent>())
				parentMatrix = parent.GetComponent<TransformComponent>().GetGlobalTransform();

			return parentMatrix * GetTransform();
		}

		Vector3 GlobalPosition() const
		{
			auto mat = GetGlobalTransform();
			return mat.Position();
		}

		Quaternion GlobalRotation() const
		{
			auto mat = GetGlobalTransform();
			return mat.Rotation();
		}

		template<typename Archive>
		void serialize(Archive& archive) 
		{
			archive(KEEP_NAME(position), KEEP_NAME(scale), KEEP_NAME(rotation), KEEP_NAME(parent));
		}
	};
	RE_REGISTER_COMPONENT(TransformComponent, "Transform")

	struct CameraComponent
	{
		float fov = 70.0f;
		float zNear = 0.1f;
		float zFar = 100.0f;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(fov), KEEP_NAME(zNear), KEEP_NAME(zFar));
		}
	};
	RE_REGISTER_COMPONENT(CameraComponent, "Camera")

	struct SkyboxComponent
	{
		Asset<Material> material;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(material));
		}
	};
	RE_REGISTER_COMPONENT(SkyboxComponent, "Skybox")

	struct PointLightComponent
	{
		Color color;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(color));
		}
	};
	RE_REGISTER_COMPONENT(PointLightComponent, "PointLight")

	struct DirectionalLightComponent
	{
		Color color;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(color));
		}
	};
	RE_REGISTER_COMPONENT(DirectionalLightComponent, "DirectionalLight")


	struct SpotLightComponent
	{
		Color color;
		float cutOff = 5.0f;
		float outerCutOff = 10.0f;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(color), KEEP_NAME(cutOff), KEEP_NAME(outerCutOff));
		}
	};
	RE_REGISTER_COMPONENT(SpotLightComponent, "SpotLight")

	// This holds the guid of the scripts on this entity
	struct ScriptComponent
	{
		const auto& GetScripts() const { return m_scripts; }

		void AddScript(const std::string& name) { m_scripts.push_back(name); }

		template <class Archive>
		void save(Archive& archive) const
		{
			// Save the list of lua tables
			
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(CUSTOM_NAME(m_scripts, "ScriptGuids"));

			// Load the scripts from the string
		}

	private:
		std::vector<std::string> m_scripts;
	};
	RE_REGISTER_COMPONENT(ScriptComponent, "Script")
}