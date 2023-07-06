#pragma once

#include <unordered_map>
#include <unordered_set>

#include <entt/entity/registry.hpp>

#include "Entity.h"
#include "../core/Serialization.h"
#include "../assets/AssetManager.h"

namespace RexEngine
{
	class Scene
	{
	public:

		RE_DECL_EVENT(OnSceneStart, Asset<Scene>)
		RE_DECL_EVENT(OnSceneStop, Asset<Scene>)

		Scene(const Guid& guid);
		~Scene();

		Scene(const Scene&) = delete;

		inline static std::shared_ptr<Scene> CreateScene() { return std::make_shared<Scene>(Guid::Generate()); }

		Entity CreateEntity(const std::string& name = "Unnamed");

		void DestroyEntity(Entity e, bool destroyChildren = false);

		// Returns a null Entity if no owner is found
		template<typename T>
		inline Entity GetComponentOwner(const T& component)
		{
			return Entity(&m_registry, entt::to_entity(m_registry, component));
		}

		// Usage : for(auto&&[entity, component] : GetComponents<T>())
		template<typename T>
		std::vector<std::pair<Entity, T&>> GetComponents()
		{
			std::vector<std::pair<Entity, T&>> result;
			auto view = m_registry.view<T>();
			for (auto entity : view)
			{
				result.push_back(std::pair<Entity, T&>{ Entity(&m_registry, entity), view.get<T>(entity)});
			}

			return result;
		}

		Guid GetGuid() const { return m_guid; }


		inline static Asset<Scene> CurrentScene()
		{
			return s_currentScene;
		}

		inline static void SetCurrentScene(Asset<Scene> scene)
		{
			s_currentScene = scene; // TODO : do scene transition/load/unload
		}

		// The the guid of the scene the entity is in
		inline static Guid GetEntityScene(const Guid& guid)
		{
			if (auto f = s_entities.find(guid); f != s_entities.end())
				return std::get<0>(f->second);
			else
				return Guid::Empty;
		}

		template<typename Archive>
		static std::shared_ptr<Scene> LoadFromAssetFile(Guid assetGuid, [[maybe_unused]]Archive& metaDataArchive, std::istream& assetFile)
		{
			auto scene = std::make_shared<Scene>(assetGuid);

			s_registryLoading = &scene->m_registry;
			scene->DeserializeJson(assetFile);
			s_registryLoading = nullptr;
			
			return scene;
		}
		 
		template<typename Archive>
		void SaveToAssetFile([[maybe_unused]] Archive& metaDataArchive, std::ostream& assetFile) const
		{
			SerializeJson(assetFile);
		}

	private:
		void SerializeJson(std::ostream& output) const;
		void DeserializeJson(std::istream& input);

		// Called when a guid is added, add this entity to the cache
		inline static void OnGuidAdded(Guid sceneGuid, entt::registry& registry, entt::entity handle)
		{
			s_entities.insert({ registry.get<Guid>(handle), {sceneGuid, handle} });
		}

		inline static void OnGuidRemoved(entt::registry& registry, entt::entity handle)
		{
			s_entities.erase(registry.get<Guid>(handle));
		}

		inline static void OnStop()
		{
			// Unload the current scene
			SetCurrentScene(Asset<Scene>());

			// Remove all entities
			s_entities.clear();
		}

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStop().Register<&Scene::OnStop>();
		});

	private:
		friend class Entity;
		// Used in Entity
		inline static bool IsEntityValid(const Guid& guid, entt::registry* registry)
		{
			// Check if the entity exists and if the registry is valid
			return s_entities.contains(guid) && s_validRegistries.contains(registry);
		}

		inline static entt::entity GetEntityHandle(const Guid& guid)
		{
			if (auto f = s_entities.find(guid); f != s_entities.end())
				return std::get<1>(f->second);
			else
				return entt::null;
		}

		entt::registry* GetRegistry() { return &m_registry; }

		inline static entt::registry* GetLoadingRegistry() { return s_registryLoading; }

	private:
		entt::registry m_registry;
		Guid m_guid;

		inline static Asset<Scene> s_currentScene;
		// <entity guid, <scene guid, entity handle>>
		inline static std::unordered_map<Guid, std::tuple<Guid, entt::entity>> s_entities;
		inline static std::unordered_set<entt::registry*> s_validRegistries;

		// This is set by a scene while loading, entities that get loaded in this scene use this as their registry
		// Should be set back to nullptr after loading
		inline static entt::registry* s_registryLoading = nullptr;
	};
}