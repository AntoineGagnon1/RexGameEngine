#pragma once

#include "../core/Guid.h"
#include "../core/Assert.h"
#include "../core/Serialization.h"

#include <entt/entt.hpp>

namespace RexEngine
{
	struct TransformComponent;
	struct TagComponent;

	// Holds the id of an entity,
	// Use the bool operator to check if the entity is null or was deleted
	// Entities should be created by the Scene, not by calling Entity()
	class Entity
	{
	private:
		friend class Scene;
		friend class SceneManager;

		Entity(entt::registry* registry, entt::entity handle)
			: m_registry(registry), m_handle(handle), m_entityGuid(Guid::Empty)
		{
			m_entityGuid = m_registry->get<Guid>(m_handle); // Dont use GetComponent<>() because the entity is not valid yet
			AssertValid();
		}

	public:
		Entity(const Guid& guid);
		Entity() : Entity(Guid::Empty) {}
		Entity(const Entity& from) = default;
		
		// Is the entity valid and has not been deleted ?
		operator bool() const;

		Guid GetGuid() const;

		std::string& Name();
		const std::string& Name() const;

		TransformComponent& Transform();
		const TransformComponent& Transform() const;

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

		template<>
		decltype(auto) AddComponent<Guid>() = delete; // All entities already have a guid
		template<>
		decltype(auto) AddComponent<TransformComponent>() = delete; // All entities already have a TransformComponent
		template<>
		decltype(auto) AddComponent<TagComponent>() = delete; // All entities already have a TagComponent

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
		template<>
		bool RemoveComponent<TransformComponent>() = delete; // Cannot delete the transform
		template<>
		bool RemoveComponent<TagComponent>() = delete; // Cannot delete the tag

		inline friend auto operator<=>(const Entity& left, const Entity& right)
		{
			return left.m_entityGuid <=> right.m_entityGuid;
		}

		inline friend bool operator==(const Entity& left, const Entity& right) { return left.m_entityGuid == right.m_entityGuid; }
		inline friend bool operator!=(const Entity& left, const Entity& right) { return !(left == right); }


		template<class Archive>
		void save(Archive& archive) const
		{
			archive(CUSTOM_NAME(m_entityGuid, "Guid"));
		}

		template<class Archive>
		void load(Archive& archive)
		{
			archive(CUSTOM_NAME(m_entityGuid, "Guid"));
			
			// Get the handle and the registry
			InitFromGuid();
		}

	private:
		void AssertValid() const;
		void InitFromGuid();

	private:
		// Used to detect if the entity was deleted, because the Guid component
		// will set itself to 0 on deletion
		Guid m_entityGuid;

		entt::entity m_handle;
		entt::registry* m_registry; // cache
	};
}