#pragma once

#include <unordered_map>

#include "Scene.h"
#include "../events/EngineEvents.h"

namespace RexEngine
{
	class SceneManager
	{
	public:

		inline static Scene CreateScene()
		{
			auto pair = s_scenes.emplace(Guid::Generate(), entt::registry());

			pair.first->second.on_construct<Guid>().connect<&SceneManager::OnGuidAdded>(pair.first->first);
			pair.first->second.on_destroy<Guid>().connect<&SceneManager::OnGuidRemoved>();

			return Scene(&pair.first->second, pair.first->first);
		}

		inline static void DeleteScene(const Scene& scene)
		{
			DeleteScene(scene.GetGuid());
		}

		inline static Scene CurrentScene()
		{
			return s_currentScene;
		}

		inline static void SetCurrentScene(const Scene& scene)
		{
			s_currentScene = scene; // TODO : do scene transition/load/unload
		}


	private:

		friend class Scene;
		friend class Entity;
		friend class ScriptEngine;

		inline static void DeleteScene(const Guid& guid)
		{
			s_scenes.erase(guid);
		}

		inline static entt::registry& GetSceneRegistry(const Guid& guid)
		{
			return s_scenes.find(guid)->second;
		}

		inline static bool IsSceneValid(const Guid& guid)
		{
			return s_scenes.contains(guid);
		}

		inline static entt::entity GetEntityHandle(const Guid& e)
		{
			if (auto f = s_entities.find(e); f != s_entities.end())
				return std::get<1>(f->second);
			else
				return entt::null;
		}

		inline static Guid GetEntitySceneGuid(const Guid& e)
		{
			if (auto f = s_entities.find(e); f != s_entities.end())
				return std::get<0>(f->second);
			else
				return Guid::Empty;
		}

	private:

		// Called when a guid is added, add this entity to the cache
		inline static void OnGuidAdded(Guid sceneGuid, entt::registry& registry, entt::entity handle)
		{
			s_entities.insert({ registry.get<Guid>(handle), {sceneGuid, handle}});
		}

		inline static void OnGuidRemoved(entt::registry& registry, entt::entity handle)
		{
			s_entities.erase(registry.get<Guid>(handle));
		}

		inline static void OnStop()
		{
			SetCurrentScene(Scene(nullptr, Guid::Empty));

			// Remove all entities
			s_entities.clear();

			// Remove (and delete) all scenes
			s_scenes.clear();
		}

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStop().Register<&SceneManager::OnStop>();
		});

	private:
		inline static std::unordered_map<Guid, entt::registry> s_scenes;
		inline static Scene s_currentScene = Scene(nullptr, Guid::Empty);

		// <entity guid, <scene guid, entity handle>>
		inline static std::unordered_map<Guid, std::tuple<Guid, entt::entity>> s_entities;
	};
}