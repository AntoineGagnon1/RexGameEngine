#include "REPch.h"
#include "Scene.h"

#include "SceneManager.h"

namespace RexEngine
{

    Entity Scene::CreateEntity()
    {
        RE_ASSERT(IsValid(), "Trying to use an invalid Scene !");

        auto handle = m_registry->create();
        Guid guid = Guid::Generate();

        SceneManager::RegisterEntity(guid, m_guid, handle);

        m_registry->emplace<Guid>(handle, guid); // All entities must have a Guid
        return Entity(m_registry, handle);
    }

    void Scene::DestroyEntity(Entity e)
    {
        RE_ASSERT(IsValid(), "Trying to use an invalid Scene !");


        // Set the Guid to 0, this is IMPORTANT because this
        // is how the entity class detects deletions
        e.GetComponent<Guid>().Reset();

        SceneManager::DeleteEntity(e.m_entityGuid);

        m_registry->destroy(e.m_handle);
    }

    bool Scene::IsValid() const
    {
        return SceneManager::IsSceneValid(m_guid);
    }
}