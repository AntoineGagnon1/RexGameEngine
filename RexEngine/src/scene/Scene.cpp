#include "REPch.h"
#include "Scene.h"

#include "SceneManager.h"
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

    Entity Scene::CreateEntity(const std::string& name)
    {
        RE_ASSERT(IsValid(), "Trying to use an invalid Scene !");

        auto handle = m_registry->create();
        Guid guid = Guid::Generate();
		TagComponent tag{ name };

        m_registry->emplace<Guid>(handle, guid); // All entities must have a Guid, TagComponent and Transform
        m_registry->emplace<TagComponent>(handle, tag);
        m_registry->emplace<TransformComponent>(handle); 

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
        Internal::OutputArchive<JsonSerializer> archive(serializer);

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
		Internal::InputArchive<JsonDeserializer> archive(deserializer);

        entt::snapshot_loader{ *m_registry }.entities(archive).component<Guid, // Guid first, this is important because the guid.on_connect event is used to notify the scenemanager that an entity has been added
                                                                         TransformComponent,
                                                                         MeshRendererComponent,
                                                                         CameraComponent,
                                                                         SkyboxComponent>(archive);
    }
}