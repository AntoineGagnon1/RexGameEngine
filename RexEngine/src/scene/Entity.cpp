#include "REPch.h"
#include "Entity.h"

#include "SceneManager.h"

namespace RexEngine
{
	Entity::Entity(const Guid& guid)
		: m_entityGuid(guid)
	{
		InitFromGuid();
	}

	Entity::operator bool() const
	{
		// This will check if the entity is valid implicitly because GetEntitySceneGuid() will return an empty guid
		return SceneManager::IsSceneValid(SceneManager::GetEntitySceneGuid(m_entityGuid));
	}

	Guid Entity::GetGuid() const
	{
		return m_entityGuid;
	}


	void Entity::AssertValid() const
	{
		RE_ASSERT(*this, "Trying to use an invalid Entity !");
	}

	void Entity::InitFromGuid()
	{
		m_handle = SceneManager::GetEntityHandle(m_entityGuid);
		if (auto scene = SceneManager::GetEntitySceneGuid(m_entityGuid); scene != Guid::Empty)
			m_registry = &SceneManager::GetSceneRegistry(scene);
		else
			m_registry = nullptr;
	}
}
