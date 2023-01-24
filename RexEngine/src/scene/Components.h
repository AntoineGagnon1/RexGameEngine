#pragma once

#include <memory>
#include <typeindex>
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

namespace RexEngine
{
	class Components
	{
	private:
		struct ComponentInfo
		{
			std::type_index type;
			std::function<bool(const Entity&)> HasComponent;
			std::function<bool(Entity&)> RemoveComponent;
			std::function<void(entt::snapshot&, RexEngine::Internal::OutputArchive<JsonSerializer>&)> SaveJson;
			std::function<void(entt::snapshot_loader&, RexEngine::Internal::InputArchive<JsonDeserializer>&)> LoadJson;
		};

	public:

		template<typename T>
		inline static void RegisterComponent()
		{
			ComponentInfo info{
				typeid(T),
				[](const Entity& e) { return e.HasComponent<T>(); }, 
				nullptr,
				[](entt::snapshot& s, RexEngine::Internal::OutputArchive<JsonSerializer>& a) { s.component<T>(a); },
				[](entt::snapshot_loader& s, RexEngine::Internal::InputArchive<JsonDeserializer>& a) { s.component<T>(a); }
			};

			// I can't find a way to check if the function is deleted :( maybe someday : https://en.cppreference.com/w/cpp/experimental/reflect
			if constexpr (std::is_same_v<T, TransformComponent> || std::is_same_v<T, TagComponent> || std::is_same_v<T, Guid>)
			{
				info.RemoveComponent = []([[maybe_unused]] Entity& e) {
					return false;
				};
			}
			else
			{
				info.RemoveComponent = [](Entity& e) {
					return e.RemoveComponent<T>();
				};
			}
			
			ComponentsVector().emplace_back(std::move(info));
		}

		// Returns a vector with all the type_index registered
		inline static auto GetComponents() 
		{
			std::vector<std::type_index> keys;
			keys.reserve(ComponentsVector().size());
			
			for (auto& c : ComponentsVector())
				keys.push_back(c.type);

			return keys;
		}

	private:

		friend class Entity;
		inline static bool EntityHasComponent(const Entity& e, std::type_index componentType)
		{
			for (auto& c : ComponentsVector())
			{
				if (c.type == componentType)
					return c.HasComponent(e);
			}

			return false;
		}
		inline static bool EntityRemoveComponent(Entity& e, std::type_index componentType)
		{
			for (auto& c : ComponentsVector())
			{
				if (c.type == componentType)
					return c.RemoveComponent(e);
			}

			return false;
		}

		friend class Scene;
		inline static void SaveJson(entt::snapshot& s, RexEngine::Internal::OutputArchive<JsonSerializer>& a, std::type_index componentType)
		{
			for (auto& c : ComponentsVector())
			{
				if (c.type == componentType)
				{
					c.SaveJson(s, a);
					return;
				}
			}
		}
		inline static void LoadJson(entt::snapshot_loader& s, RexEngine::Internal::InputArchive<JsonDeserializer>& a, std::type_index componentType)
		{
			for (auto& c : ComponentsVector())
			{
				if (c.type == componentType)
				{
					c.LoadJson(s, a);
					return;
				}
			}
		}

		static void StaticConstructor();
		
		RE_STATIC_CONSTRUCTOR({
			StaticConstructor();
		})

		inline static std::vector<ComponentInfo>& ComponentsVector()
		{ // Lazy init
			static std::vector<ComponentInfo> components;
			return components;
		}
	};


	struct TagComponent
	{
		std::string name;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(name));
		}
	};

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

	struct SkyboxComponent
	{
		Asset<Material> material;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(material));
		}
	};

	struct PointLightComponent
	{
		Color color;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(color));
		}
	};

	struct DirectionalLightComponent
	{
		Color color;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(color));
		}
	};

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

	// This holds the guid of the scripts on this entity
	struct ScriptComponent
	{
		std::vector<Guid> GetScriptGuids() const { return scripts; }


		template <class Archive>
		void save(const Archive& archive) const
		{
			// Save the guid list
			archive(CUSTOM_NAME(scripts, "ScriptGuids"));

			// Save the string generated by the c# engine

		}

		template <class Archive>
		void load(const Archive& archive)
		{
			archive(CUSTOM_NAME(scripts, "ScriptGuids"));

			// Load the scripts from the string
		}

	private:
		std::vector<Guid> scripts;
	};
}