#include <REPch.h>
#include "ForwardRenderer.h"

#include "RenderQueue.h"
#include "RenderCommands.h"
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
 		SceneDataUniforms sceneData{viewMatrix, projectionMatrix, cameraPos};

		auto sceneDataBuffer = UniformBlocks::GetBlock<SceneDataUniforms>("SceneData").GetBufferID();
		RenderApi::SubBufferData(sceneDataBuffer, RenderApi::BufferType::Uniforms, 0, sizeof(SceneDataUniforms), &sceneData);

		
		// Update the lighting data

		// Point lights and Directional lights
		std::vector<LightData> lights;

		for (auto&& [e, c] : scene->GetComponents<PointLightComponent>())
			lights.emplace_back(e.Transform().GlobalPosition(), (Vector3)c.color, false);

		for (auto&& [e, c] : scene->GetComponents<DirectionalLightComponent>())
			lights.emplace_back(e.Transform().GlobalForward(), (Vector3)c.color, true);

		auto lightBufferID = UniformBlocks::GetBlock<LightData>("LightsData").GetBufferID();

		if (lights.size() > LightsMax)
		{ // Resize the buffer
			LightsMax = (uint32_t)(LightsMax * 1.5f) + 1;
			RenderApi::SetBufferData(lightBufferID, RenderApi::BufferType::ShaderStorage, RenderApi::BufferMode::Dynamic, nullptr, sizeof(LightData) * lights.size() + sizeof(uint32_t) + 12); // 12 is the padding
		}

		uint32_t LightsCount = static_cast<uint32_t>(lights.size());
		RenderApi::SubBufferData(lightBufferID, RenderApi::BufferType::ShaderStorage, 0, sizeof(uint32_t), &LightsCount);
		if(LightsMax > 0)
			RenderApi::SubBufferData(lightBufferID, RenderApi::BufferType::ShaderStorage, sizeof(uint32_t) + 12, sizeof(LightData) * lights.size(), lights.data()); // 12 is the padding


		// Spot lights
		std::vector<SpotLightData> spotLights;

		for (auto&& [e, c] : scene->GetComponents<SpotLightComponent>())
			spotLights.emplace_back(e.Transform().GlobalPosition(), e.Transform().GlobalForward(), (Vector3)c.color, std::cos(glm::radians(c.cutOff)), std::cos(glm::radians(c.outerCutOff)));

		auto spotLightBufferID = UniformBlocks::GetBlock<SpotLightData>("SpotLightsData").GetBufferID();
		if (spotLights.size() > SpotLightsMax)
		{ // Resize the buffer
			SpotLightsMax = (uint32_t)(SpotLightsMax * 1.5f) + 1;
			RenderApi::SetBufferData(spotLightBufferID, RenderApi::BufferType::ShaderStorage, RenderApi::BufferMode::Dynamic, nullptr, sizeof(SpotLightData) * spotLights.size() + sizeof(uint32_t) + 12); // 12 is the padding
		}

		uint32_t SpotLightsCount = static_cast<uint32_t>(spotLights.size());
		RenderApi::SubBufferData(spotLightBufferID, RenderApi::BufferType::ShaderStorage, 0, sizeof(uint32_t), &SpotLightsCount);
		if (SpotLightsMax > 0)
			RenderApi::SubBufferData(spotLightBufferID, RenderApi::BufferType::ShaderStorage, sizeof(uint32_t) + 12, sizeof(SpotLightData) * spotLights.size(), spotLights.data()); // 12 is the padding


		auto& opaqueQueue = RenderQueues::GetQueue<OpaqueRenderCommand>("Opaque");

		// Skybox TODO : render this after the opaque objects, but before the transparent ones
		// Get the skybox component
		auto&& skyboxes = scene->GetComponents<SkyboxComponent>();
		if (skyboxes.size() > 1)
			RE_LOG_WARN("Multiple skyboxes active !");

		if (skyboxes.size() >= 1)
		{
			auto skyboxMesh = Shapes::GetCubeMesh();
			auto& c = skyboxes[0].second;

			if (c.material && c.material->GetShader())
				opaqueQueue.AddCommand<OpaqueRenderCommand>(c.material, skyboxMesh, Matrix4::MakeTransform(cameraPos, Quaternion::Identity(), {1,1,1})); // Remove the translation (the skybox is always around the player)
		}


		// Draw objects (put them in the RenderQueue)
		for (auto&& [e, c] : scene->GetComponents<MeshRendererComponent>())
		{
			const Matrix4 modelMatrix = e.GetComponent<TransformComponent>().GetGlobalTransform();
			
			if(c.material && c.mesh && c.material->GetShader()) // Has a material, a shader and a mesh
				opaqueQueue.AddCommand<OpaqueRenderCommand>(c.material, c.mesh, modelMatrix);
		}
	}
}