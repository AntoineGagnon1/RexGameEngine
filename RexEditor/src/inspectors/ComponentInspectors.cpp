#include <REDPch.h>
#include "../panels/Inspector.h"

namespace RexEditor::ComponentInspectors
{
	void InspectMeshRenderer(RexEngine::Entity entity)
	{
		auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
		UI::AssetInput<Material> shader("Material", meshRenderer.material);
		UI::AssetInput<Mesh>       mesh("Mesh    ", meshRenderer.mesh);
	}

	void InspectCamera(RexEngine::Entity entity)
	{
		auto& camera = entity.GetComponent<CameraComponent>();
		UI::FloatInput   fov("Fov  ", camera.fov);
		UI::FloatInput znear("zNear", camera.zNear);
		UI::FloatInput  zfar("zFar ", camera.zFar);
	}

	void InspectSkybox(RexEngine::Entity entity)
	{
		auto& skybox = entity.GetComponent<SkyboxComponent>();
		UI::AssetInput<Material> shader("Material", skybox.material);
	}

	void InspectPointLight(RexEngine::Entity entity)
	{
		auto& light = entity.GetComponent<PointLightComponent>();

		// Split the color in a normalized color (from 0 to 1) and an intensity
		float intensity = Scalar::Max(Scalar::Max(1.0f, light.color.r), Scalar::Max(light.color.g, light.color.b));
		Color normalizedColor = Color(light.color.r / intensity, light.color.g / intensity, light.color.b / intensity);

		UI::ColorInput colInput("Color", normalizedColor, false);

		UI::FloatInput intensityInput("Intensity", intensity);

		if (colInput.HasChanged() || intensityInput.HasChanged())
			light.color = Color(normalizedColor.r * intensity, normalizedColor.g * intensity, normalizedColor.b * intensity);
	}

	void InspectDirectionalLight(RexEngine::Entity entity)
	{
		auto& light = entity.GetComponent<DirectionalLightComponent>();

		// Split the color in a normalized color (from 0 to 1) and an intensity
		float intensity = Scalar::Max(Scalar::Max(1.0f, light.color.r), Scalar::Max(light.color.g, light.color.b));
		Color normalizedColor = Color(light.color.r / intensity, light.color.g / intensity, light.color.b / intensity);

		UI::ColorInput colInput("Color", normalizedColor, false);

		UI::FloatInput intensityInput("Intensity", intensity);

		if (colInput.HasChanged() || intensityInput.HasChanged())
			light.color = Color(normalizedColor.r * intensity, normalizedColor.g * intensity, normalizedColor.b * intensity);
	}

	void InspectSpotLight(RexEngine::Entity entity)
	{
		auto& light = entity.GetComponent<SpotLightComponent>();

		// Split the color in a normalized color (from 0 to 1) and an intensity
		float intensity = Scalar::Max(Scalar::Max(1.0f, light.color.r), Scalar::Max(light.color.g, light.color.b));
		Color normalizedColor = Color(light.color.r / intensity, light.color.g / intensity, light.color.b / intensity);

		UI::ColorInput colInput("Color", normalizedColor, false);

		UI::FloatInput intensityInput("Intensity", intensity);

		if (colInput.HasChanged() || intensityInput.HasChanged())
			light.color = Color(normalizedColor.r * intensity, normalizedColor.g * intensity, normalizedColor.b * intensity);

		UI::FloatInput      cutOff("CutOff      ", light.cutOff);
		UI::FloatInput outerCutOff("Outer CutOff", light.outerCutOff);
	}

	RE_STATIC_CONSTRUCTOR({
		InspectorPanel::ComponentInspectors().Add<MeshRendererComponent>(InspectMeshRenderer);
		InspectorPanel::ComponentInspectors().Add<CameraComponent>(InspectCamera);
		InspectorPanel::ComponentInspectors().Add<SkyboxComponent>(InspectSkybox);
		InspectorPanel::ComponentInspectors().Add<PointLightComponent>(InspectPointLight);
		InspectorPanel::ComponentInspectors().Add<DirectionalLightComponent>(InspectDirectionalLight);
		InspectorPanel::ComponentInspectors().Add<SpotLightComponent>(InspectSpotLight);
	})
}