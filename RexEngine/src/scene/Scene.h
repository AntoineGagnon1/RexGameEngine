#pragma once

#include <entt/entity/registry.hpp>

#include "Entity.h"
#include "Components.h"
#include "../core/Serialization.h"

namespace RexEngine
{
	class Scene
	{
	private:
		friend class SceneManager;

		Scene(entt::registry* registry, Guid guid) : m_registry(registry), m_guid(guid) {}

	public:

		Scene() : Scene(nullptr, Guid::Empty) {}

		Scene(const Scene&) = default;

		Entity CreateEntity();

		void DestroyEntity(Entity e);

		// Returns a null Entity if no owner is found
		template<typename T>
		inline Entity GetComponentOwner(const T& component)
		{
			RE_ASSERT(IsValid(), "Trying to use an invalid Scene !");
			return Entity(m_registry, entt::to_entity(*m_registry, component));
		}

		// Usage : for(auto&&[entity, component] : GetComponents<T>())
		template<typename T>
		std::vector<std::pair<Entity, T&>> GetComponents()
		{
			RE_ASSERT(IsValid(), "Trying to use an invalid Scene !");
			std::vector<std::pair<Entity, T&>> result;
			auto view = m_registry->view<T>();
			for (auto entity : view)
			{
				result.push_back(std::pair<Entity, T&>{ Entity(m_registry, entity), view.get<T>(entity)});
			}

			return result;
		}

		void SerializeJson(std::ostream& output) const;

		void DeserializeJson(std::istream& input);

		Guid GetGuid() const { return m_guid; }

		bool IsValid() const;

	private:
		entt::registry* m_registry;
		Guid m_guid;

	};
}