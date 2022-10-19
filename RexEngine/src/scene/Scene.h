#pragma once
#include <entt/entity/registry.hpp>

#include "Entity.h"

namespace RexEngine
{
	class Scene
	{
	public:
		Entity CreateEntity();

		void DestroyEntity(Entity e);

	private:
		entt::registry m_registry;

	};
}