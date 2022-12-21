#include <REPch.h>
#include "ForwardRenderer.h"

#include "RenderQueue.h"
#include "Shapes.h"

namespace RexEngine
{
	void ForwardRenderer::RenderScene(Asset<Scene> scene, const CameraComponent& camera)
	{
		if (!scene) // No scene
			return;

		// Get the transform of the camera
		Entity cameraOwner = scene->GetComponentOwner<const CameraComponent>(camera);

		auto& cameraTransform = cameraOwner.Transform();
		
		// Make the view matrix
		auto[cameraPos, cameraRotation, _] = cameraTransform.GetGlobalTransform().Decompose();
		auto viewMatrix = Matrix4::MakeLookAt(cameraPos, cameraPos + (cameraRotation * Directions::Forward), Directions::Up);

		// Projection matrix
		auto viewport = RenderApi::GetViewportSize();
		auto projectionMatrix = Matrix4::MakePerspective(camera.fov, (float)viewport.x / (float)viewport.y, camera.zNear, camera.zFar);


		// Update the scene data for the uniform blocks
 		SceneDataUniforms newSceneData{viewMatrix, projectionMatrix, cameraPos};
		RenderApi::SubBufferData(GetSceneDataUniforms(), RenderApi::BufferType::Uniforms, 0, sizeof(SceneDataUniforms), &newSceneData);
		
		// Update the lighting data
		LightingUniforms newLighting{ Vector3(10,10, -10), 0.0f, Vector3(10.0f,10.0f,10.0f)}; // Manually set a white light at 10,10,-10
		RenderApi::SubBufferData(GetLightingUniforms(), RenderApi::BufferType::Uniforms, 0, sizeof(LightingUniforms), &newLighting);


		// Draw objects (put them in the RenderQueue)
		for (auto&& [e, c] : scene->GetComponents<MeshRendererComponent>())
		{
			const Matrix4 modelMatrix = e.GetComponent<TransformComponent>().GetGlobalTransform();
			
			if(c.material && c.mesh) // Has a material and a mesh
				RenderQueue::AddCommand(RenderCommand(c.material, c.mesh, modelMatrix));
		}

		// Execute the render queue to actually render the objects on the screen
		RenderApi::SetDepthFunction(RenderApi::DepthFunction::Less);
		RenderQueue::ExecuteCommands();
		
		// Skybox
		
		// Get the skybox component
		auto&& skyboxes = scene->GetComponents<SkyboxComponent>();
		if (skyboxes.size() > 1)
			RE_LOG_WARN("Multiple skyboxes active !");

		if (skyboxes.size() >= 1)
		{
			auto skyboxMesh = Shapes::GetCubeMesh();
			auto& c = skyboxes[0].second;

			// New uniforms for this shader
			newSceneData.worldToView = Matrix4(Matrix3(newSceneData.worldToView)); // Remove the translation (the skybox is always around the player)
			RenderApi::SubBufferData(GetSceneDataUniforms(), RenderApi::BufferType::Uniforms, 0, sizeof(SceneDataUniforms), &newSceneData);
			
			if(c.material)
				RenderQueue::AddCommand(RenderCommand(c.material, skyboxMesh, Matrix4::Identity));
		}
		// Render the skybox
		RenderApi::SetDepthFunction(RenderApi::DepthFunction::LessEqual);
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

	RenderApi::BufferID ForwardRenderer::GetLightingUniforms()
	{
		static RenderApi::BufferID uniforms = []() {
			auto buf = RenderApi::MakeBuffer();
			auto data = LightingUniforms();
			RenderApi::SetBufferData(buf, RenderApi::BufferType::Uniforms, RenderApi::BufferMode::Dynamic, (uint8_t*)&data, sizeof(LightingUniforms));
			RenderApi::BindBufferBase(buf, RenderApi::BufferType::Uniforms, 3);

			return buf;
		}();

		return uniforms;
	}
}