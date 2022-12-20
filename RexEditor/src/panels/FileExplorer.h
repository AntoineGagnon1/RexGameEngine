#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "Panel.h"
#include "../project/ProjectManager.h"

#include "../core/EditorAssets.h"
#include "../ui/DragDrop.h"
#include "../ui/MenuSystem.h"
#include "../inspectors/AssetInspector.h"
#include "Inspector.h"

namespace RexEditor
{
	class FileExplorerPanel : public Panel
	{
	public:
		FileExplorerPanel() : Panel("File Explorer")
		{
			EditorEvents::OnLoadProject().Register<&FileExplorerPanel::OnProjectLoad>(this);
		}

		~FileExplorerPanel()
		{
			EditorEvents::OnLoadProject().UnRegister<&FileExplorerPanel::OnProjectLoad>(this);
		}

	protected:
		virtual void OnGui(float deltaTime) override
		{
			if (!std::filesystem::exists(m_currentFolder))
				return;

			
			// Back arrow, dont go back if alredy at the root
			if (UI::Button b("<-"); b.IsClicked() && m_currentFolder != ProjectManager::CurrentProject().rootPath)
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
			int itemWidth = 64 * m_scale;
			int cols = (int)floor((float)PanelSize().x / (float)(itemWidth + 8));
			cols = RexEngine::Scalar::Clamp(cols, 0, 64); // imgui needs a value between 0 and 64

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
							m_currentFolder = entry.path();
					}
					else // File
					{
						auto extension = entry.path().extension().string();
						if (extension != Asset<int>::FileExtension) // Dont show .asset metadata files
						{
							UI::Icon icon(entry.path().filename().string(), 
								EditorAssets::FileIcon(),
								{ itemWidth ,itemWidth }
							);

							// Get the asset type
							auto type = RexEngine::AssetTypes::GetAssetTypeFromExtension(extension);
							if (!type.Empty())
							{
								if (icon.IsClicked(MouseButton::Left, UI::MouseAction::Released)) // Tell the inspector
								{
									InspectorPanel::InspectElement(std::bind(&AssetInspector::InspectAsset,
										std::placeholders::_1, type, entry.path()));
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

				return m;
			}();

			return menu;
		}

	private:

		void OnProjectLoad(Project p)
		{
			m_currentFolder = ProjectManager::CurrentProject().rootPath;
		}

	private:
		std::filesystem::path m_currentFolder;
		float m_scale = 1.5f;
	};
}