#include "REPch.h"
#include "Entity.h"

#include "Components.h"
#include "Scene.h"

namespace RexEngine
{
	Entity::Entity(const Guid& guid)
		: m_entityGuid(guid)
	{
		InitFromGuid();
	}

	Entity::operator bool() const
	{
		return Scene::IsEntityValid(m_entityGuid, m_registry);
	}

	Guid Entity::GetGuid() const
	{
		return m_entityGuid;
	}

	std::string& Entity::Name()
	{
		return GetComponent<TagComponent>().name;
	}

	const std::string& Entity::Name() const
	{
		return GetComponent<TagComponent>().name;
	}

	TransformComponent& Entity::Transform()
	{
		return GetComponent<TransformComponent>();
	}

	const TransformComponent& Entity::Transform() const
	{
		return GetComponent<TransformComponent>();
	}

	size_t Entity::GetComponentCount() const
	{
		size_t count = 0;

		for (auto& c : ComponentFactories::GetFactories())
		{
			if (c->HasComponent(*this))
				count++;
		}

		return count;
	}

	bool Entity::HasComponent(std::type_index type) const
	{
		return ComponentFactories::GetFactory(type)->HasComponent(*this);
	}

	bool Entity::RemoveComponent(std::type_index type)
	{
		return ComponentFactories::GetFactory(type)->RemoveComponent(*this);
	}

	void Entity::AssertValid() const
	{
		RE_ASSERT(*this, "Trying to use an invalid Entity !");
	}

	void Entity::InitFromGuid()
	{
		if (m_entityGuid == Guid::Empty)
		{
			m_handle = entt::null;
			m_registry = nullptr;
			return;
		}

		m_handle = Scene::GetEntityHandle(m_entityGuid);

		if (Scene::GetLoadingRegistry() != nullptr)
			m_registry = Scene::GetLoadingRegistry();
		else
		{ // Not loading a scene, get the registry using the asset manager
			auto scene = AssetManager::GetAsset<Scene>(Scene::GetEntityScene(m_entityGuid));
			if (scene)
				m_registry = scene->GetRegistry();
			else
				m_registry = nullptr;
		}
	}
}
