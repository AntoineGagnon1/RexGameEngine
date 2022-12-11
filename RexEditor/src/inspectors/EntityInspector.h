#pragma once

#include <RexEngine.h>
#include "../ui/UI.h"
#include "../ui/UIElements.h"

#include <typeinfo>

namespace RexEditor
{
	template<typename T>
	static void TryDrawComponent(RexEngine::Entity);

	inline void InspectEntity(float deltaTime, RexEngine::Entity entity)
	{
		using namespace RexEngine;

		if (!entity)
		{
			UI::Text t("Invalid Entity !");
			return;
		}

		// Header
		TryDrawComponent<TagComponent>(entity);

		// Transform
		TryDrawComponent<TransformComponent>(entity);

		// Other components
		TryDrawComponent<SkyboxComponent>(entity);
		TryDrawComponent<CameraComponent>(entity);
		TryDrawComponent<MeshRendererComponent>(entity);
	}

	// Returns true if the component is visible (add a separator after)
	template<typename T>
	static bool DrawComponent(RexEngine::Entity entity);

	template<typename T>
	inline static void TryDrawComponent(RexEngine::Entity entity)
	{
		if (entity && entity.HasComponent<T>())
		{
			if(DrawComponent<T>(entity)) // Is open ?
				UI::Separator();
		}
	}

	// Will also draw the guid
	template<>
	inline static bool DrawComponent<RexEngine::TagComponent>(RexEngine::Entity entity)
	{
		UI::Anchor a(UI::AnchorPos::Center);
		UI::PushFontScale(UI::FontScale::Large);
		UI::TextInput nameText("##name", 50, entity.Name());
		UI::PopFontScale();

		UI::PushFontScale(UI::FontScale::Small);
		UI::Text guidText(entity.GetGuid().ToString());
		UI::PopFontScale();

		return true;
	}

	template<>
	inline static bool DrawComponent<RexEngine::TransformComponent>(RexEngine::Entity entity)
	{
		UI::Vector3Input posInput("Position", entity.Transform().position);
		
		// Convert to and from euler angles
		Vector3 angles = entity.Transform().rotation.EulerAngles();
		UI::Vector3Input Rotation("Rotation", angles);
		entity.Transform().rotation = Quaternion::FromEuler(angles);


		UI::Vector3Input posScale("Scale   ", entity.Transform().scale);

		return true;
	}

	template<>
	inline static bool DrawComponent<RexEngine::MeshRendererComponent>(RexEngine::Entity entity)
	{
		if(UI::TreeNode n("Mesh Renderer", UI::TreeNodeFlags::CollapsingHeader); n.IsOpen()) 
		{
			auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
			UI::AssetInput<Shader> shader("Shader", meshRenderer.shader);
			UI::AssetInput<Mesh>     mesh("Mesh  ", meshRenderer.mesh);
			UI::ComboBoxEnum<RenderApi::CullingMode> cullingMode("Render faces", { "Front", "Back", "Both" }, meshRenderer.cullingMode);
			UI::ByteInput priority("Priority", meshRenderer.priority);
			return true;
		}

		return false;
	}

	template<>
	inline static bool DrawComponent<RexEngine::CameraComponent>(RexEngine::Entity entity)
	{
		if (UI::TreeNode n("Camera", UI::TreeNodeFlags::CollapsingHeader); n.IsOpen())
		{
			auto& camera = entity.GetComponent<CameraComponent>();
			UI::FloatInput   fov("Fov  ", camera.fov);
			UI::FloatInput znear("zNear", camera.zNear);
			UI::FloatInput  zfar("zFar ", camera.zFar);
			return true;
		}

		return false;
	}

	template<>
	inline static bool DrawComponent<RexEngine::SkyboxComponent>(RexEngine::Entity entity)
	{
		if (UI::TreeNode n("Skybox", UI::TreeNodeFlags::CollapsingHeader); n.IsOpen())
		{
			auto& skybox = entity.GetComponent<SkyboxComponent>();
			UI::AssetInput<Shader> shader("Shader", skybox.shader);
			return true;
		}

		return false;
	}
}