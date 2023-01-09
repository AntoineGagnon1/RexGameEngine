#include "REPch.h"
#include "Scene.h"

#include "Components.h"

namespace RexEngine::Internal
{
	// entt archives with names
	template<typename Serializer> // ex : JsonSerializer
	class OutputArchive
	{
	public:

		OutputArchive(Serializer& output) : m_output(output) {}

		// count is the number of entity that will be stored
		void operator()(std::underlying_type_t<entt::entity> count)
		{
			m_output(CUSTOM_NAME(count, "Count"));
		}

		void operator()(entt::entity e)
		{
			m_output(CUSTOM_NAME(e, "Entity"));
		}

		template<typename T>
		void operator()(entt::entity e, const T& c)
		{
			m_output(CUSTOM_NAME(e, "Owner"), CUSTOM_NAME(c, typeid(c).name()));
		}

	private:
		Serializer& m_output;
	};

	template<typename Deserializer> // ex : JsonDeserializer
	class InputArchive
	{
	public:

		InputArchive(Deserializer& input) : m_input(input) {}

		// count is the number of entity that will be stored
		void operator()(std::underlying_type_t<entt::entity>& count) const
		{
			m_input(CUSTOM_NAME(count, "Count"));
		}

		void operator()(entt::entity& e) const
		{
			m_input(CUSTOM_NAME(e, "Entity"));
		}

		template<typename T>
		void operator()(entt::entity& e, T& c) const
		{
			m_input(CUSTOM_NAME(e, "Owner"), CUSTOM_NAME(c, typeid(c).name()));
		}

	private:
		Deserializer& m_input;
	};
}

namespace RexEngine
{
	Scene::Scene(const Guid& guid)
		: m_guid(guid)
	{
		m_registry.on_construct<Guid>().connect<&Scene::OnGuidAdded>(guid);
		m_registry.on_destroy<Guid>().connect<&Scene::OnGuidRemoved>();
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

        entt::snapshot{ m_registry }.entities(archive).component<Guid, // Guid first, this is important because the guid.on_connect event is used to notify the scenemanager that an entity has been added
                                                                  TagComponent,
																  TransformComponent,
                                                                  MeshRendererComponent,
                                                                  CameraComponent,
                                                                  SkyboxComponent,
																  PointLightComponent,
																  DirectionalLightComponent,
																  SpotLightComponent>(archive);
    }

    void Scene::DeserializeJson(std::istream& input)
    {
        JsonDeserializer deserializer(input);
		Internal::InputArchive<JsonDeserializer> archive(deserializer);

        entt::snapshot_loader{ m_registry }.entities(archive).component<Guid, // Guid first, this is important because the guid.on_connect event is used to notify the scenemanager that an entity has been added
																		 TagComponent,
																		 TransformComponent,
                                                                         MeshRendererComponent,
                                                                         CameraComponent,
                                                                         SkyboxComponent,
																		 PointLightComponent,
																		 DirectionalLightComponent,
																		 SpotLightComponent>(archive);
    }
}