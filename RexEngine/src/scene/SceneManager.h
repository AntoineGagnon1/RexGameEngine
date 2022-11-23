#pragma once

#include <unordered_map>

#include "Scene.h"

namespace RexEngine
{
	class SceneManager
	{
	public:

		inline static Scene CreateScene()
		{
			auto pair = s_scenes.emplace(Guid::Generate(), entt::registry());
			return Scene(&pair.first->second, pair.first->first);
		}

		inline static bool IsSceneValid(const Guid& guid)
		{
			return s_scenes.contains(guid);
		}

		inline static void DeleteScene(const Guid& guid)
		{
			s_scenes.erase(guid);
		}

		inline static entt::registry& GetSceneRegistry(const Guid& guid)
		{
			return s_scenes.find(guid)->second;
		}

		inline static void RegisterEntity(const Guid& e, const Guid& sceneGuid, entt::entity handle)
		{
			s_entities.insert({ e, {sceneGuid, handle} });
		}

		inline static void DeleteEntity(const Guid& e)
		{
			s_entities.erase(e);
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
		inline static std::unordered_map<Guid, entt::registry> s_scenes;

		// <entity guid, <scene guid, entity handle>>
		inline static std::unordered_map<Guid, std::tuple<Guid, entt::entity>> s_entities;
	};
}