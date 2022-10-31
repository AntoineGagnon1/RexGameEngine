#include "REPch.h"
#include "Scene.h"

namespace RexEngine
{

    Entity Scene::CreateEntity()
    {
        auto handle = m_registry.create();
        m_registry.emplace<Guid>(handle, Guid::Generate()); // All entities must have a Guid
        return Entity(m_registry, handle);
    }

    void Scene::DestroyEntity(Entity e)
    {
        // Set the Guid to 0, this is IMPORTANT because this
        // is how the entity class detects deletions
        e.GetComponent<Guid>().Reset();

        m_registry.destroy(e.m_handle);
    }
}