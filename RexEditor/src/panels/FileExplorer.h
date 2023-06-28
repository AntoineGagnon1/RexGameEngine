#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "Panel.h"
#include "PanelManager.h"
#include "../project/ProjectManager.h"
#include "Inspector.h"

#include "../core/EditorAssets.h"
#include "../ui/DragDrop.h"
#include "../ui/MenuSystem.h"
#include "../utils/TypeMap.h"


namespace RexEditor
{
	class FileExplorerPanel : public Panel
	{
	public:
		FileExplorerPanel() : Panel("File Explorer")
		{
			EditorEvents::OnProjectLoaded().Register<&FileExplorerPanel::OnProjectLoad>(this);
		}

		~FileExplorerPanel()
		{
			EditorEvents::OnProjectLoaded().UnRegister<&FileExplorerPanel::OnProjectLoad>(this);
		}

	protected:
		virtual void OnGui([[maybe_unused]] float deltaTime) override
		{
			if (!std::filesystem::exists(m_currentFolder))
				return;
			
			// Back arrow, dont go back if alredy at the root
			if (UI::Button b("<-"); b.IsClicked() && m_currentFolder != ProjectManager::CurrentProject().rootPath / "Assets")
			{
				m_currentFolder = m_currentFolder.parent_path();
			}

			// Path label, start the path at the root folder
			UI::SameLine();
			// Remove everyting before the root
			auto pathString = m_currentFolder.string().substr(ProjectManager::CurrentProject().rootPath.parent_path().string().size());
			RexEngine::StringHelper::ReplaceChars(pathString, '\\', '/'); // Replace all \ with /
			UI::Text pathText(pathString);

			// Calculate the number of columns
			float itemWidth = floor(64.0f * m_scale);
			int cols = (int)floor((float)PanelSize().x / (itemWidth + (8.0f * 2.0f)));
			cols = RexEngine::Scalar::Clamp(cols, 1, 64); // imgui needs a value between 1 and 64

			// The file names should be small
			UI::PushFontScale(UI::FontScale::Small);

			
			if (UI::Table table("FileExplorerTable", cols); table.IsVisible())
			{
				table.SetCellPadding({ 8,8 });
				for (auto& entry : std::filesystem::directory_iterator(m_currentFolder))
				{
					table.NextElement();
					if (entry.is_directory())
					{ // Folder
						UI::Icon icon(entry.path().filename().string(), EditorAssets::FolderIcon(), {itemWidth ,itemWidth});
						
						if (icon.IsDoubleClicked()) // go into the folder
						{
							m_currentFolder = entry.path();
							m_canInspect = false;
							break;
						}
					}
					else // File
					{
						auto extension = entry.path().extension().string();
						if (extension != AssetFileExtension) // Dont show .asset metadata files
						{
							// Get the asset type
							auto type = RexEngine::AssetTypes::GetAssetTypeFromExtension(extension);
							auto guid = AssetManager::GetAssetGuidFromPath(entry.path());
							
							const Texture* iconTexture = &EditorAssets::FileIcon();

							if (IconRegistry().Contains(type.type)) // Check for special icons
							{
								iconTexture = &IconRegistry().Get(type.type)(guid);
							}

							UI::Icon icon(entry.path().filename().string(),
								*iconTexture,
								{ itemWidth , itemWidth }
							);

							if (!type.Empty())
							{
								if (icon.IsClicked(MouseButton::Left, UI::MouseAction::Clicked))
									m_canInspect = true;

								if (icon.IsClicked(MouseButton::Left, UI::MouseAction::Released) 
									&& m_canInspect) // Debounce, don't inspect if the folder just changed
								{ // Tell the inspector
									if(auto panel = PanelManager::GetPanel<InspectorPanel>(); panel != nullptr)
										panel->InspectAsset(type.type, guid);
								}

								// TODO : double click : do something based on the file type

								// Asset drag and drop
								UI::DragDrop::Source<std::filesystem::path>("Asset" + type.name,
									entry.path(),
									entry.path().filename().string());
							}
						}
						else
						{
							table.PreviousElement(); // Revert this element, because it is empty
						}
					}
				}
			}

			UI::PopFontScale();


			// Scale slider
			{
				UI::Anchor a(UI::AnchorPos::BottomRight);
				UI::FloatSlider("Scale :", 1.0f, 2.0f, 64, m_scale);
			}

			if (UI::ContextMenu context("FileExplorerContext"); context.IsOpen())
			{
				ContextMenu().DrawMenu();
			}
		}

	public:
		inline static UI::MenuSystem<>& ContextMenu()
		{
			static auto menu = [] {
				UI::MenuSystem<> m;
				// Default elements
				m.AddMenuItem("Create/Material", [] { 
					auto path = SystemDialogs::SaveFile("New Material", SystemDialogs::GetAssetTypeFilter<Material>());
					path.replace_extension(RexEngine::AssetTypes::GetAssetType<Material>().extensions[0]);
					
					if (!path.empty())
					{
						// TODO : change the shader to the PBR shader
						auto mat = std::make_shared<Material>(Asset<Shader>());

						// Add it to the asset manager
						Guid guid = Guid::Generate();
						RexEngine::AssetManager::AddAsset<Material>(guid, path, Asset(guid, mat));
					}
				});

				m.AddMenuItem("Create/Cubemap", [] {
					auto path = SystemDialogs::SaveFile("New Cubemap", SystemDialogs::GetAssetTypeFilter<Cubemap>());
					path.replace_extension(RexEngine::AssetTypes::GetAssetType<Cubemap>().extensions[0]);

					if (!path.empty())
					{
						auto cubemap = std::make_shared<Cubemap>(Asset<Texture>(), 128, Cubemap::ProjectionMode::HDRI);

						// Add it to the asset manager
						Guid guid = Guid::Generate();
						RexEngine::AssetManager::AddAsset<Cubemap>(guid, path, Asset(guid, cubemap));
					}
				});

				m.AddMenuItem("Create/C# Script", [] {
					auto path = SystemDialogs::SaveFile("New C# Script", SystemDialogs::GetAssetTypeFilter<ScriptAsset>());
					path.replace_extension(RexEngine::AssetTypes::GetAssetType<ScriptAsset>().extensions[0]);

					if (!path.empty())
					{
						ScriptAsset::CreateNewScript(path, ProjectManager::CurrentProject().Name);
						ProjectManager::CurrentProject().RegisterScript(path);
						auto script = std::make_shared<ScriptAsset>();

						// Add it to the asset manager
						Guid guid = Guid::Generate();
						RexEngine::AssetManager::AddAsset<ScriptAsset>(guid, path, Asset(guid, script));
					}
				});

				return m;
			}();

			return menu;
		}

		// std::function<iconID(asset guid)>
		inline static TypeMap<std::function<const Texture& (Guid)>>& IconRegistry()
		{
			static TypeMap<std::function<const Texture& (Guid)>> map;
			return map;
		}

	private:

		void OnProjectLoad(Project p)
		{
			m_currentFolder = ProjectManager::CurrentProject().rootPath / "Assets";
		}

	private:
		std::filesystem::path m_currentFolder;
		bool m_canInspect = true;
		float m_scale = 1.5f;
	};
}