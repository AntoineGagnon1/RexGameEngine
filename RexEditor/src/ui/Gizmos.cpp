#include "REDPch.h"
#include "Gizmos.h"

#include "core/EditorEvents.h"
#include "core/EditorAssets.h"

namespace RexEditor
{
	void Gizmos::Billboard(const RexEngine::Texture& texture, const RexEngine::Vector3& position, const RexEngine::Vector3& size)
	{
		static auto textureIndex = TextureManager::ReserveSlot();
		s_billboardMaterial->GetShader()->SetUniformInt("texture", textureIndex);
		RenderApi::SetActiveTexture(textureIndex);
		texture.Bind();

		// size * 0.5 because the Quad is 2x2x2
		RenderQueues::GetQueue<TransparentRenderCommand>("Gizmos")
			.AddCommand<TransparentRenderCommand>(
				s_billboardMaterial, 
				Shapes::GetQuadMesh(), 
				Matrix4::MakeTransform(position, Quaternion::Identity(), size * 0.5f),
				s_cameraPos
			);
	}

	void Gizmos::DrawGizmos(const TransformComponent& camera)
	{
		auto [cameraPos, cameraRotation, _] = camera.GetGlobalTransform().Decompose();
		auto viewMatrix = Matrix4::MakeLookAt(cameraPos, cameraPos + (cameraRotation * Directions::Forward), Directions::Up);

		const Vector3 right = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
		const Vector3 up = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };

		s_billboardMaterial->GetUniform<Vector3>("CameraRightWS") = right;
		s_billboardMaterial->GetUniform<Vector3>("CameraUpWS") = up;

		s_cameraPos = cameraPos;

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