#include "REPch.h"
#include "Entity.h"

namespace RexEngine
{
	Entity::Entity(entt::registry& registry, entt::entity handle)
		: m_registry(&registry), 
		  m_handle(handle), 
		  m_guid(m_registry->get<Guid>(m_handle))
	{	}

	Entity::operator bool() const
	{
		// Check if the handle is valid and if the guid is still the same (the entity wasn't deleted)
		return m_registry != nullptr && m_registry->valid(m_handle) && m_guid == m_registry->get<Guid>(m_handle);
	}

	Guid Entity::GetGuid() const
	{
		AssertValid();
		return m_guid;
	}


	void Entity::AssertValid() const
	{
		RE_ASSERT(*this, "Trying to use an invalid Entity !")
	}
}
