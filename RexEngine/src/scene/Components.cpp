#include <REPch.h>
#include "Components.h"

namespace RexEngine
{
	void Components::StaticConstructor()
	{
		// The order is important (entt save/load)
		RegisterComponent<Guid>();
		RegisterComponent<TagComponent>();
		RegisterComponent<TransformComponent>();
		RegisterComponent<MeshRendererComponent>();
		RegisterComponent<CameraComponent>();
		RegisterComponent<SkyboxComponent>();
		RegisterComponent<PointLightComponent>();
		RegisterComponent<DirectionalLightComponent>();
		RegisterComponent<SpotLightComponent>();
		//RegisterComponent<ScriptComponent>();
	}
}