#pragma once

#include <unordered_map>

#include "Scene.h"
#include "../events/EngineEvents.h"

namespace RexEngine
{
	/*
	class SceneManager
	{
	private:

		inline static entt::registry& GetSceneRegistry(const Guid& guid)
		{
			return s_scenes.find(guid)->second;
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
	};*/
}