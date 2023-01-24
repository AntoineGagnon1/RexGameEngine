#include "REPch.h"
#include "Scene.h"

#include "Components.h"


namespace RexEngine
{
	Scene::Scene(const Guid& guid)
		: m_guid(guid)
	{
		m_registry.on_construct<Guid>().connect<&Scene::OnGuidAdded>(guid);
		m_registry.on_destroy<Guid>().connect<&Scene::OnGuidRemoved>();
		s_validRegistries.insert(&m_registry);
	}

	Scene::~Scene()
	{
		s_validRegistries.erase(&m_registry);
	}

    Entity Scene::CreateEntity(const std::string& name)
    {
        auto handle = m_registry.create();
        Guid guid = Guid::Generate();
		TagComponent tag{ name };

        m_registry.emplace<Guid>(handle, guid); // All entities must have a Guid, TagComponent and Transform
        m_registry.emplace<TagComponent>(handle, tag);
        m_registry.emplace<TransformComponent>(handle); 

        return Entity(&m_registry, handle);
    }

    void Scene::DestroyEntity(Entity e, bool destroyChildren)
    {
		// TODO : cache parent/child ?
		for (auto& [child, t] : GetComponents<TransformComponent>())
		{
			if (t.parent == e && child != e)
			{
				if (destroyChildren)
				{ // Destroy the child
					DestroyEntity(child, true);
				}
				else
				{ // Make the parent of the child the root
					t.parent = Entity();
				}
			}
		}

        m_registry.destroy(e.m_handle);
    }

    void Scene::SerializeJson(std::ostream& output) const
    {
        JsonSerializer serializer(output);
        Internal::OutputArchive<JsonSerializer> archive(serializer);

		auto snapshot = entt::snapshot{ m_registry };
		snapshot.entities(archive);
		snapshot.component<Guid>(archive); // Guid first, this is important because the guid.on_connect event is used to notify the scenemanager that an entity has been added

		for (auto& c : Components::GetComponents())
		{
			if(c != typeid(Guid))
				Components::SaveJson(snapshot, archive, c);
		}
    }

    void Scene::DeserializeJson(std::istream& input)
    {
        JsonDeserializer deserializer(input);
		Internal::InputArchive<JsonDeserializer> archive(deserializer);

		auto snapshot = entt::snapshot_loader{ m_registry };
		snapshot.entities(archive);
		snapshot.component<Guid>(archive); // Guid first, this is important because the guid.on_connect event is used to notify the scenemanager that an entity has been added

		for (auto& c : Components::GetComponents())
		{
			if (c != typeid(Guid))
				Components::LoadJson(snapshot, archive, c);
		}
    }
}