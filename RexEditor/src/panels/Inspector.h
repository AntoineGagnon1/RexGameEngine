#pragma once

#include <RexEngine.h>

#include <typeindex>

#include "Panel.h"
#include "../core/EditorEvents.h"
#include "../utils/TypeMap.h"
#include "ui/MenuSystem.h"

namespace RexEditor
{
	class InspectorPanel : public Panel
	{
	public:
		using InspectGuiCallback = std::function<void()>;

	public:
		InspectorPanel() : Panel("Inspector")
		{ }


		void InspectCustomElement(InspectGuiCallback callback)
		{
			m_currentElement = callback;
		}

		void InspectAsset(std::type_index assetType, const Guid& assetGuid)
		{
			InspectCustomElement([assetType, assetGuid] (){
				if (InspectorPanel::AssetInspectors().Contains(assetType))
				{
					{
						auto path = AssetManager::GetAssetPathFromGuid(assetGuid);

						UI::Anchor a(UI::AnchorPos::Center);
						UI::PushFontScale(UI::FontScale::Large);
						UI::Text title(path.filename().string());
						UI::Separator();
						UI::PopFontScale();
					}

					// Not in the centered anchor
					UI::EmptyLine(); 
					AssetInspectors().Get(assetType)(assetGuid);
				}
				else
				{
					UI::Text("No inspector found for this asset type !");
				}
			});
		}

		void InspectEntity(const Guid& entityGuid)
		{
			InspectCustomElement([entityGuid] {
				using namespace RexEngine;

				auto entity = Entity(entityGuid);

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
				for (auto& c : ComponentFactories::GetFactories())
				{
					TryDrawComponent(c->GetType(), entity);
				}

				// Script components
				if (entity.HasComponent<ScriptComponent>())
				{
					for (auto& script : entity.GetComponent<ScriptComponent>().Scripts())
					{
						auto name = script.GetClass().Name();
						UI::TreeNode n(name + std::format("##{}", (intptr_t)script.GetPtr()), UI::TreeNodeFlags::CollapsingHeader | UI::TreeNodeFlags::DefaultOpen);
						if (n.IsOpen())
						{
							for (auto& field : script.GetSerializedFields())
							{
								auto type = field.Type();
								if (type == typeid(int))
								{
									int value = script.GetValue<int>(field);
									if (UI::IntInput input(field.Name(), value); input.HasChanged())
									{
										script.SetValue<int>(field, value);
									}
								}
							}
						}
						UI::Separator();

						if (n.IsClicked(RexEngine::MouseButton::Right))
						{
							UI::Popup::Open("ComponentContextMenu" + name);
						}

						if (UI::Popup p("ComponentContextMenu" + name); p.IsOpen())
						{
							if (UI::MenuItem i("Remove " + name); i.IsClicked())
								entity.GetComponent<ScriptComponent>().RemoveScript(script);
						}
					}
				}

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
					if (UI::Menu m("Scripts"); m.IsOpen())
					{
						auto& types = MonoApi::ScriptTypes();
						for (auto& type : types)
						{
							if (UI::MenuItem i(type->GetClass().Name()); i.IsClicked())
							{
								if (!entity.HasComponent<ScriptComponent>())
									entity.AddComponent<ScriptComponent>();

								entity.GetComponent<ScriptComponent>().AddScript(type);
							}
						}
					}
				}
			});
		}

		// std::function<void(asset guid)>
		inline static TypeMap<std::function<void(const Guid&)>>& AssetInspectors()
		{
			static TypeMap<std::function<void(const Guid&)>> map;
			return map;
		}

		// std::function<void(entity guid)>
		inline static TypeMap<std::function<void(const Guid&)>>& ComponentInspectors()
		{
			static TypeMap<std::function<void(const Guid&)>> map;
			return map;
		}

		inline static UI::MenuSystem<RexEngine::Entity>& NewComponentMenu()
		{
			static UI::MenuSystem<RexEngine::Entity> menu = [] {
				UI::MenuSystem<RexEngine::Entity> m;

				// Default components
				m.AddMenuItem("Rendering/Mesh Renderer", [](RexEngine::Entity e) { TryAddComponent<MeshRendererComponent>(e); });
				m.AddMenuItem("Rendering/Camera", [](RexEngine::Entity e) { TryAddComponent<CameraComponent>(e); });
				m.AddMenuItem("Environment/Skybox", [](RexEngine::Entity e) { TryAddComponent<SkyboxComponent>(e); });
				m.AddMenuItem("Lighting/Point Light", [](RexEngine::Entity e) { TryAddComponent<PointLightComponent>(e); });
				m.AddMenuItem("Lighting/Directional Light", [](RexEngine::Entity e) { TryAddComponent<DirectionalLightComponent>(e); });
				m.AddMenuItem("Lighting/Spot Light", [](RexEngine::Entity e) { TryAddComponent<SpotLightComponent>(e); });

				return m;
			}();

			return menu;
		}

	protected:
		virtual void OnGui([[maybe_unused]]float deltaTime) override
		{
			if (m_currentElement)
				m_currentElement();
		}

	private:

		// Add the component if the entity does not already have it
		template<typename T>
		inline static void TryAddComponent(Entity e)
		{
			if (e && !e.HasComponent<T>())
				e.AddComponent<T>();
		}

		inline static void TryDrawComponent(std::type_index type, RexEngine::Entity entity)
		{
			// The tag and the transform are already drawn
			if (type == typeid(TransformComponent) || type == typeid(TagComponent) || type == typeid(Guid) || type == typeid(ScriptComponent))
				return;

			std::string label = type.name();

			if (auto found = label.find_last_of("::"); found != std::string::npos)
				label = label.substr(found + 1); // Remove the namespaces

			if (auto found = label.find_last_of("Component"); found != std::string::npos)
				label = label.substr(0, found - 8); // Remove the Component posfix

			if (entity && entity.HasComponent(type))
			{
				UI::TreeNode n(label, UI::TreeNodeFlags::CollapsingHeader | UI::TreeNodeFlags::DefaultOpen);
				if (n.IsOpen())
				{
					if (InspectorPanel::ComponentInspectors().Contains(type))
					{
						InspectorPanel::ComponentInspectors().Get(type)(entity.GetGuid());
					}
				}
				UI::Separator();

				if (n.IsClicked(RexEngine::MouseButton::Right))
				{
					UI::Popup::Open("ComponentContextMenu" + label);
				}

				if (UI::Popup p("ComponentContextMenu" + label); p.IsOpen())
				{
					if (UI::MenuItem i("Remove " + label); i.IsClicked())
						entity.RemoveComponent(type);
				}

			}
		}

	private:
		InspectGuiCallback m_currentElement;

	};
}