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
		
		// Update the lighting data

		// Point lights and Directional lights
		std::vector<LightData> lights;

		for (auto&& [e, c] : scene->GetComponents<PointLightComponent>())
			lights.emplace_back(e.Transform().GlobalPosition(), (Vector3)c.color, false);

		for (auto&& [e, c] : scene->GetComponents<DirectionalLightComponent>())
			lights.emplace_back(e.Transform().GlobalForward(), (Vector3)c.color, true);

		if (lights.size() > LightsMax)
		{ // Resize the buffer
			LightsMax = (uint32_t)(LightsMax * 1.5f) + 1;
			RenderApi::SetBufferData(GetLightsBuffer(), RenderApi::BufferType::ShaderStorage, RenderApi::BufferMode::Dynamic, nullptr, sizeof(LightData) * lights.size() + sizeof(uint32_t) + 12); // 12 is the padding
		}

		uint32_t LightsCount = lights.size();
		RenderApi::SubBufferData(GetLightsBuffer(), RenderApi::BufferType::ShaderStorage, 0, sizeof(uint32_t), &LightsCount);
		if(LightsMax > 0)
			RenderApi::SubBufferData(GetLightsBuffer(), RenderApi::BufferType::ShaderStorage, sizeof(uint32_t) + 12, sizeof(LightData) * lights.size(), lights.data()); // 12 is the padding


		// Spot lights
		std::vector<SpotLightData> spotLights;

		for (auto&& [e, c] : scene->GetComponents<SpotLightComponent>())
			spotLights.emplace_back(e.Transform().GlobalPosition(), e.Transform().GlobalForward(), (Vector3)c.color, std::cos(glm::radians(c.cutOff)), std::cos(glm::radians(c.outerCutOff)));

		if (spotLights.size() > SpotLightsMax)
		{ // Resize the buffer
			SpotLightsMax = (uint32_t)(SpotLightsMax * 1.5f) + 1;
			RenderApi::SetBufferData(GetSpotLightsBuffer(), RenderApi::BufferType::ShaderStorage, RenderApi::BufferMode::Dynamic, nullptr, sizeof(SpotLightData) * spotLights.size() + sizeof(uint32_t) + 12); // 12 is the padding
		}

		uint32_t SpotLightsCount = spotLights.size();
		RenderApi::SubBufferData(GetSpotLightsBuffer(), RenderApi::BufferType::ShaderStorage, 0, sizeof(uint32_t), &SpotLightsCount);
		if (SpotLightsMax > 0)
			RenderApi::SubBufferData(GetSpotLightsBuffer(), RenderApi::BufferType::ShaderStorage, sizeof(uint32_t) + 12, sizeof(SpotLightData) * spotLights.size(), spotLights.data()); // 12 is the padding


		// Skybox TODO : render this after the opaque objects, but before the transparent ones
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

			if (c.material)
				RenderQueue::AddCommand(RenderCommand(c.material, skyboxMesh, Matrix4::Identity));
		}
		// Render the skybox
		RenderQueue::ExecuteCommands();

		// Revert to the complete matrix
		newSceneData.worldToView = viewMatrix;
		RenderApi::SubBufferData(GetSceneDataUniforms(), RenderApi::BufferType::Uniforms, 0, sizeof(SceneDataUniforms), &newSceneData);


		// Draw objects (put them in the RenderQueue)
		for (auto&& [e, c] : scene->GetComponents<MeshRendererComponent>())
		{
			const Matrix4 modelMatrix = e.GetComponent<TransformComponent>().GetGlobalTransform();
			
			if(c.material && c.mesh && c.material->GetShader()) // Has a material, a shader and a mesh
				RenderQueue::AddCommand(RenderCommand(c.material, c.mesh, modelMatrix));
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

	RenderApi::BufferID ForwardRenderer::GetLightsBuffer()
	{
		static RenderApi::BufferID uniforms = []() {
			auto buf = RenderApi::MakeBuffer();
			RenderApi::SetBufferData(buf, RenderApi::BufferType::ShaderStorage, RenderApi::BufferMode::Dynamic, (uint8_t*)&LightsMax, sizeof(uint32_t));
			RenderApi::BindBufferBase(buf, RenderApi::BufferType::ShaderStorage, 3);
			return buf;
		}();

		return uniforms;
	}

	RenderApi::BufferID ForwardRenderer::GetSpotLightsBuffer()
	{
		static RenderApi::BufferID uniforms = []() {
			auto buf = RenderApi::MakeBuffer();
			RenderApi::SetBufferData(buf, RenderApi::BufferType::ShaderStorage, RenderApi::BufferMode::Dynamic, (uint8_t*)&SpotLightsMax, sizeof(uint32_t));
			RenderApi::BindBufferBase(buf, RenderApi::BufferType::ShaderStorage, 4);
			return buf;
		}();

		return uniforms;
	}
}