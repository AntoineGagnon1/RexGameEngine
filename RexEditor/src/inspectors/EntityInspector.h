#pragma once

#include <RexEngine.h>
#include "../ui/UI.h"
#include "../ui/UIElements.h"
#include "../ui/MenuSystem.h"

#include <typeinfo>

namespace RexEditor
{
	class EntityInspector
	{
	public:
		inline static void InspectEntity(float deltaTime, RexEngine::Entity entity)
		{
			using namespace RexEngine;

			if (!entity)
			{
				UI::Text t("Invalid Entity !");
				return;
			}

			// Header
			{
				UI::Anchor a(UI::AnchorPos::Center);
				UI::PushFontScale(UI::FontScale::Large);
				UI::TextInput nameText("##name", 50, entity.Name());
				UI::PopFontScale();

				UI::PushFontScale(UI::FontScale::Small);
				UI::Text guidText(entity.GetGuid().ToString());
				UI::PopFontScale();
			}

			UI::Separator();
			// Transform
			UI::Vector3Input posInput("Position", entity.Transform().position);

			// Convert to and from euler angles
			Vector3 angles = entity.Transform().rotation.EulerAngles();
			UI::Vector3Input Rotation("Rotation", angles);
			entity.Transform().rotation = Quaternion::FromEuler(angles);

			UI::Vector3Input posScale("Scale   ", entity.Transform().scale);
			UI::Separator();

			// Other components
			TryDrawComponent<SkyboxComponent>("Skybox", entity);
			TryDrawComponent<CameraComponent>("Camera", entity);
			TryDrawComponent<MeshRendererComponent>("Mesh Renderer", entity);

			// Add components
			{
				UI::Anchor a(UI::AnchorPos::Center);
				UI::EmptyLine();
				if (UI::Button addComponent("New Component"); addComponent.IsClicked())
				{
					UI::Popup::Open("NewComponentPopup");
				}
			}

			if (UI::Popup p("NewComponentPopup"); p.IsOpen())
			{
				NewComponentMenu().DrawMenu(entity);
			}
		}

		inline static UI::MenuSystem<RexEngine::Entity>& NewComponentMenu()
		{
			static UI::MenuSystem<RexEngine::Entity> menu = [] {
				UI::MenuSystem<RexEngine::Entity> m;

				// Default components
				m.AddMenuItem("Rendering/Mesh Renderer", [](RexEngine::Entity e) { TryAddComponent<MeshRendererComponent>(e); });
				m.AddMenuItem("Rendering/Camera", [](RexEngine::Entity e) { TryAddComponent<CameraComponent>(e); });
				m.AddMenuItem("Environment/Skybox", [](RexEngine::Entity e) { TryAddComponent<SkyboxComponent>(e); });

				return m; 
			}();

			return menu;
		}

	private:

		// Add the component if the entity does not already have it
		template<typename T>
		inline static void TryAddComponent(Entity e)
		{
			if (e && !e.HasComponent<T>())
				e.AddComponent<T>();
		}

		// Returns true if the component is visible (add a separator after)
		template<typename T>
		static void DrawComponent(RexEngine::Entity entity);

		template<typename T>
		inline static void TryDrawComponent(const std::string& label, RexEngine::Entity entity)
		{
			if (entity && entity.HasComponent<T>())
			{
				UI::TreeNode n(label, UI::TreeNodeFlags::CollapsingHeader | UI::TreeNodeFlags::DefaultOpen);
				if (n.IsOpen())
				{
					DrawComponent<T>(entity);
				}
				UI::Separator();

				if(n.IsClicked(RexEngine::MouseButton::Right))
				{
					UI::Popup::Open("ComponentContextMenu" + label);
				}

				if (UI::Popup p("ComponentContextMenu" + label); p.IsOpen())
				{
					if(UI::MenuItem i("Remove " + label); i.IsClicked())
						entity.RemoveComponent<T>();
				}

			}
		}

		template<>
		inline static void DrawComponent<RexEngine::MeshRendererComponent>(RexEngine::Entity entity)
		{
			auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
			UI::AssetInput<Shader> shader("Shader", meshRenderer.shader);
			UI::AssetInput<Mesh>     mesh("Mesh  ", meshRenderer.mesh);
			UI::ComboBoxEnum<RenderApi::CullingMode> cullingMode("Render faces", { "Front", "Back", "Both" }, meshRenderer.cullingMode);
			UI::ByteInput priority("Priority", meshRenderer.priority);
		}

		template<>
		inline static void DrawComponent<RexEngine::CameraComponent>(RexEngine::Entity entity)
		{
			auto& camera = entity.GetComponent<CameraComponent>();
			UI::FloatInput   fov("Fov  ", camera.fov);
			UI::FloatInput znear("zNear", camera.zNear);
			UI::FloatInput  zfar("zFar ", camera.zFar);
		}

		template<>
		inline static void DrawComponent<RexEngine::SkyboxComponent>(RexEngine::Entity entity)
		{
			auto& skybox = entity.GetComponent<SkyboxComponent>();
			UI::AssetInput<Shader> shader("Shader", skybox.shader);
		}
	};
}