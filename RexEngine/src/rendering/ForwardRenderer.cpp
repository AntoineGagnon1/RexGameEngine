#include <REPch.h>
#include "ForwardRenderer.h"

#include "RenderQueue.h"

namespace RexEngine
{
	void ForwardRenderer::RenderScene(Scene& scene, const CameraComponent& camera)
	{
		// Get the transform of the camera
		Entity cameraOwner = scene.GetComponentOwner<const CameraComponent>(camera);
		RE_ASSERT(cameraOwner.HasComponent<TransformComponent>(), "Camera has no TransformComponent!");

		auto& cameraTransform = cameraOwner.GetComponent<TransformComponent>();
		
		// Make the view matrix
		auto[cameraPos, cameraRotation, _] = cameraTransform.GetGlobalTransform().Decompose();
		auto viewMatrix = Matrix4::MakeLookAt(cameraPos, cameraPos + cameraRotation * Vector3(1,0,0), Vector3(0, 1, 0));

		// Projection matrix
		auto viewport = RenderApi::GetViewportSize();
		auto projectionMatrix = Matrix4::MakePerspective(camera.fov, viewport.y / viewport.x, camera.zNear, camera.zFar);

		// Draw objects (put them in the RenderQueue)
		for (auto&& [e, c] : scene.GetComponents<MeshRendererComponent>())
		{
			RenderQueue::AddCommand(RenderCommand(c.shader->GetID(), c.mesh->GetID(), c.mesh->GetIndexCount(), c.priority));
		}

		// Execute the render queue to actually render the objects on the screen
		RenderQueue::ExecuteCommands();
	}
}