#pragma once

#include "../core/Guid.h"
#include "../core/Assert.h"

#include <entt/entt.hpp>

namespace RexEngine
{
	// Holds the id of an entity,
	// Use the bool operator to check if the entity is null or was deleted
	// Entities should be created by the Scene, not by calling Entity()
	class Entity
	{
	private:
		friend class Scene;
		Entity(entt::registry& registry, entt::entity handle);

	public:
		Entity() : m_registry(nullptr), m_handle(entt::null), m_guid(Guid::Empty) {}
		Entity(const Entity& from) = default;
		
		// Is the entity valid and has not been deleted ?
		operator bool() const;

		Guid GetGuid() const;

		// TODO : GetTransform
		// TODO : Get Name
		
		template<typename ...Types>
		bool HasComponents() const
		{
			AssertValid();
			return m_registry->all_of<Types...>(m_handle);
		}
		
		template<typename T>
		bool HasComponent() const { return HasComponents<T>(); }
		
		// Will trigger an assert break if the entity already had the component
		template<typename T, typename ...Args>
		decltype(auto) AddComponent(Args&&... args)
		{
			RE_ASSERT(!HasComponent<T>(), "Entity already has this component !");
			return m_registry->emplace<T>(m_handle, std::forward<Args>(args)...);
		}

		// Get the components, use HasComponents() to check if the component is there first
		template<typename ...Types>
		decltype(auto) GetComponents()
		{
			RE_ASSERT(HasComponents<Types...>(), "Entity does not have these components !");
			return m_registry->get<Types...>(m_handle);
		}

		template<typename ...Types>
		decltype(auto) GetComponents() const
		{
			RE_ASSERT(HasComponents<Types...>(), "Entity does not have these components !");
			return m_registry->get<Types...>(m_handle);
		}

		// Get the component, use HasComponent() to check if the component is there first
		template<typename T>
		T& GetComponent() { return GetComponents<T>(); }

		template<typename T>
		const T& GetComponent() const { return GetComponents<T>(); }

		// Returns true if the component was removed, false otherwise
		template<typename T>
		bool RemoveComponent()
		{
			AssertValid();
			return m_registry->remove<T>(m_handle) > 0;
		}

		template<>
		bool RemoveComponent<Guid>() = delete; // Cannot delete the Guid

		inline friend auto operator<=>(const Entity& left, const Entity& right)
		{
			return left.m_guid <=> right.m_guid;
		}

	private:
		entt::registry* m_registry;
		entt::entity m_handle;

		// Used to detect if the entity was deleted, because the Guid component
		// will set itself to 0 on deletion
		Guid m_guid;

		void AssertValid() const;
	};
}