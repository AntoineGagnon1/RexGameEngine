#include "REDPch.h"
#include "Gizmos.h"

#include "core/EditorEvents.h"
#include "core/EditorAssets.h"

namespace RexEditor
{
	void Gizmos::Billboard([[maybe_unused]]const RexEngine::Texture& texture, [[maybe_unused]] const RexEngine::Vector3& position, [[maybe_unused]] const RexEngine::Vector3& size)
	{
		// TODO : change this when upgrading the RenderQueue
		static auto textureIndex = TextureManager::ReserveSlot();
		s_billboardMaterial->GetShader()->SetUniformInt("texture", textureIndex);
		RenderApi::SetActiveTexture(textureIndex);
		texture.Bind();

		RenderQueue::AddCommand(RenderCommand(
			s_billboardMaterial,
			Shapes::GetQuadMesh(), 
			Matrix4::MakeTransform(position, Quaternion::Identity(), size * 0.5f)) // size * 0.5 because the Quad is 2x2x2
		);

		RenderQueue::ExecuteCommands();
	}

	void Gizmos::DrawGizmos([[maybe_unused]] const TransformComponent& camera)
	{
		auto [cameraPos, cameraRotation, _] = camera.GetGlobalTransform().Decompose();
		auto viewMatrix = Matrix4::MakeLookAt(cameraPos, cameraPos + (cameraRotation * Directions::Forward), Directions::Up);

		const Vector3 right = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
		const Vector3 up = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };

		s_billboardMaterial->GetUniform<Vector3>("CameraRightWS") = right;
		s_billboardMaterial->GetUniform<Vector3>("CameraUpWS") = up;

		EditorEvents::OnGizmos().Dispatch();
	}

	void Gizmos::DefaultGizmos()
	{
		auto currentScene = Scene::CurrentScene();
		if (!currentScene)
			return;

		// Camera
		for (auto& [e, camera] : currentScene->GetComponents<CameraComponent>())
			Billboard(EditorAssets::Camera(), e.Transform().position);

		// Lights
		for (auto& [e, light] : currentScene->GetComponents<PointLightComponent>())
			Billboard(EditorAssets::Bulb(), e.Transform().position);

		for (auto& [e, light] : currentScene->GetComponents<SpotLightComponent>())
			Billboard(EditorAssets::Bulb(), e.Transform().position);

	}
}