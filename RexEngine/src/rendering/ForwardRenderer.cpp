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
		auto viewMatrix = Matrix4::MakeLookAt(cameraPos, cameraPos + (cameraRotation * Directions::Forward), Directions::Up);

		// Projection matrix
		auto viewport = RenderApi::GetViewportSize();
		auto projectionMatrix = Matrix4::MakePerspective(camera.fov, viewport.x / viewport.y, camera.zNear, camera.zFar);

		// Update the scene data for the uniform blocks
		SceneDataUniforms newSceneData;
		newSceneData.viewMatrix = viewMatrix;
		newSceneData.projectionMatrix = projectionMatrix;
		RenderApi::SubBufferData(GetSceneDataUniforms(), RenderApi::BufferType::Uniforms, 0, sizeof(SceneDataUniforms), &newSceneData);

		// Draw objects (put them in the RenderQueue)
		for (auto&& [e, c] : scene.GetComponents<MeshRendererComponent>())
		{
			Matrix4 modelMatrix = Matrix4::Identity;
			if (e.HasComponent<TransformComponent>())
			{
				// Bind the model matrix
				auto modelMatrix = e.GetComponent<TransformComponent>().GetGlobalTransform();
			}


			RenderQueue::AddCommand(RenderCommand(c.shader->GetID(), c.mesh->GetID(), c.mesh->GetIndexCount(), modelMatrix, c.priority));
		}

		// Execute the render queue to actually render the objects on the screen
		RenderQueue::ExecuteCommands();
	}


	RenderApi::BufferID ForwardRenderer::GetSceneDataUniforms()
	{
		static RenderApi::BufferID uniforms = []() {
			auto buf = RenderApi::MakeBuffer();
			auto data = SceneDataUniforms();
			RenderApi::SetBufferData(buf, RenderApi::BufferType::Uniforms, RenderApi::BufferMode::Dynamic, (uint8_t*)&data, sizeof(SceneDataUniforms));
			RenderApi::BindBufferBase(buf, RenderApi::BufferType::Uniforms, 1);

			return buf;
		}();

		return uniforms;
	}
}