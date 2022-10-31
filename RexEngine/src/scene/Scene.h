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

		// Returns a null Entity if no owner is found
		template<typename T>
		inline Entity GetComponentOwner(const T& component)
		{
			return Entity(m_registry, entt::to_entity(m_registry, component));
		}

		// Usage : for(auto&&[entity, component] : GetComponents<T>())
		template<typename T>
		decltype(auto) GetComponents()
		{
			return m_registry.view<T>().each();
		}

	private:
		entt::registry m_registry;

	};
}