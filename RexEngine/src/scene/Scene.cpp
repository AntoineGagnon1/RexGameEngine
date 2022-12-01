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

        m_registry->emplace<Guid>(handle, guid); // All entities must have a Guid
        return Entity(m_registry, handle);
    }

    void Scene::DestroyEntity(Entity e)
    {
        RE_ASSERT(IsValid(), "Trying to use an invalid Scene !");

        m_registry->destroy(e.m_handle);
    }

    bool Scene::IsValid() const
    {
        return SceneManager::IsSceneValid(m_guid);
    }

    void Scene::SerializeJson(std::ostream& output) const
    {
        RE_ASSERT(IsValid(), "Trying to serialize an invalid Scene !");
        JsonSerializer serializer(output);
        OutputArchive<JsonSerializer> archive(serializer);

        entt::snapshot{ *m_registry }.entities(archive).component<Guid, // Guid first, this is important because the guid.on_connect event is used to notify the scenemanager that an entity has been added
                                                                  TransformComponent,
                                                                  MeshRendererComponent,
                                                                  CameraComponent,
                                                                  SkyboxComponent>(archive);
    }

    void Scene::DeserializeJson(std::istream& input)
    {
        RE_ASSERT(IsValid(), "Trying to deserialize into an invalid Scene !");
        JsonDeserializer deserializer(input);
        InputArchive<JsonDeserializer> archive(deserializer);

        entt::snapshot_loader{ *m_registry }.entities(archive).component<Guid, // Guid first, this is important because the guid.on_connect event is used to notify the scenemanager that an entity has been added
                                                                         TransformComponent,
                                                                         MeshRendererComponent,
                                                                         CameraComponent,
                                                                         SkyboxComponent>(archive);
    }
}