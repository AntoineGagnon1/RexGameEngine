#include <REPch.h>
#include "ForwardRenderer.h"

#include "RenderQueue.h"

namespace RexEngine
{
	void ForwardRenderer::RenderScene(Scene& scene, const CameraComponent& camera)
	{
		// Cache the skybox, this is because the mesh cannot be created between render calls
		auto& skyboxMesh = GetSkyboxMesh();

		// Get the transform of the camera
		Entity cameraOwner = scene.GetComponentOwner<const CameraComponent>(camera);
		RE_ASSERT(cameraOwner.HasComponent<TransformComponent>(), "Camera has no TransformComponent!");

		auto& cameraTransform = cameraOwner.GetComponent<TransformComponent>();
		
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
		for (auto&& [e, c] : scene.GetComponents<MeshRendererComponent>())
		{
			Matrix4 modelMatrix = Matrix4::Identity;
			if (e.HasComponent<TransformComponent>())
 				modelMatrix = e.GetComponent<TransformComponent>().GetGlobalTransform(); // Use the transform of the object


			RenderQueue::AddCommand(RenderCommand(c.shader->GetID(), c.mesh->GetID(), c.mesh->GetIndexCount(), modelMatrix, c.cullingMode, c.priority));
		}


		// Execute the render queue to actually render the objects on the screen
		RenderApi::SetDepthFunction(RenderApi::DepthFunction::Less);
		RenderQueue::ExecuteCommands();

		// Skybox
		
		// Get the skybox component
		auto&& skyboxes = scene.GetComponents<SkyboxComponent>();
		if (skyboxes.size() > 1)
			RE_LOG_WARN("Multiple skyboxes active !");

		if (skyboxes.size() >= 1)
		{
			auto& c = skyboxes[0].second;

			// New uniforms for this shader
			newSceneData.worldToView = Matrix4(Matrix3(newSceneData.worldToView)); // Remove the translation (the skybox is always around the player)
			RenderApi::SubBufferData(GetSceneDataUniforms(), RenderApi::BufferType::Uniforms, 0, sizeof(SceneDataUniforms), &newSceneData);

			RenderQueue::AddCommand(RenderCommand(c.shader->GetID(), skyboxMesh.GetID(), skyboxMesh.GetIndexCount(), Matrix4::Identity, RenderApi::CullingMode::Back, 0));
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

	const Mesh& ForwardRenderer::GetSkyboxMesh()
	{
		static std::vector<Vector3> vertices = {
				{-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}, { 1 ,1,-1}, { 1,-1,-1}, {-1,-1,-1},
				{ 1,-1,-1}, { 1, 1,-1}, { 1, 1, 1}, { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1},
				{-1,-1, 1}, {-1, 1, 1}, {-1, 1,-1}, {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1},
				{ 1, 1, 1}, {-1, 1, 1}, {-1,-1, 1}, {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1},
				{-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}, { 1, 1, 1}, { 1, 1,-1}, {-1, 1,-1},
				{-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1}, { 1,-1,-1}, { 1,-1, 1}, {-1,-1, 1}
		};

		static std::vector<unsigned int> indices = {
			0,1,2, 3,4,5, 6,7,8, 9,10,11, 12,13,14, 15,16,17, 18,19,20, 21,22,23, 24,25,26, 27,28,29, 30,31,32, 33,34,35
		};

		static Mesh mesh(vertices, indices);

		return mesh;
	}
}