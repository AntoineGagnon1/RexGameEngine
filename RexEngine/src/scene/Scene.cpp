#include "REPch.h"
#include "Scene.h"

#include "Components.h"


namespace RexEngine
{
	Scene::Scene(const Guid& guid)
		: m_guid(guid)
	{
		m_registry.on_construct<Guid>().connect<&Scene::OnGuidAdded>(m_guid);
		m_registry.on_destroy<Guid>().connect<&Scene::OnGuidRemoved>();
		s_validRegistries.insert(&m_registry);
	}

	Scene::~Scene()
	{
		m_registry.clear(); // Allow the entities to be removed from s_entities
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
        JsonSerializer archive(output);

		archive(CUSTOM_NAME(m_registry.alive(), "EntityCount"));

		// For each entity
		m_registry.each([&archive, this](auto handle) {
			auto constRegistry = &m_registry;
			entt::registry* registry = (entt::registry*)((void*)constRegistry); // Cast the const away, bad but the entity will only be used in a const way
			const Entity e = Entity(registry, handle);

			archive(CUSTOM_NAME(e.GetGuid(), "Guid"));

			archive(CUSTOM_NAME(e.GetComponentCount(), "ComponentCount"));

			// Loop all component types, save the 
			for (auto& c : ComponentFactories::GetFactories())
			{
				if (c->HasComponent(e))
				{
					archive(CUSTOM_NAME(c->GetName(), "Type"));

					c->ToJson(e, archive);
				}
			}
		});
    }

    void Scene::DeserializeJson(std::istream& input)
    {
        JsonDeserializer archive(input);

		size_t nbEntities;
		archive(CUSTOM_NAME(nbEntities, "EntityCount"));
		
		for (auto i = 0; i < nbEntities; i++)
		{
			// Do the Guid first, to create the entity
			Guid guid;
			archive(CUSTOM_NAME(guid, "Guid"));

			// Create the entity
			auto handle = m_registry.create();
			m_registry.emplace<Guid>(handle, guid);
			// These components can't be added via the Entity class, so add them now
			m_registry.emplace<TagComponent>(handle);
			m_registry.emplace<TransformComponent>(handle);

			Entity e = Entity(&m_registry, handle);

			// Load the components
			size_t nbComponents;
			archive(CUSTOM_NAME(nbComponents, "ComponentCount"));

			for (auto j = 0; j < nbComponents; j++)
			{
				// Get the name of the component
				std::string name;
				archive(CUSTOM_NAME(name, "Type"));

				auto factory = ComponentFactories::GetFactory(name);
				if (factory != nullptr)
				{
					factory->FromJson(e, archive);
				}
				else
				{
					RE_LOG_WARN("Component type {} not found", name);
				}

			}
		}
    }
}