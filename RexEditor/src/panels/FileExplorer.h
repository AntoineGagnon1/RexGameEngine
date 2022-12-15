#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "Panel.h"
#include "project/ProjectManager.h"

#include "core/EditorAssets.h"
#include "../ui/DragDrop.h"

namespace RexEditor
{
	class FileExplorerPanel : public Panel
	{
	private:
		struct File
		{
			std::string name;
			// TODO : icon
		};

		struct Folder
		{
			std::string name;

			std::vector<File> files;
			std::vector<Folder> subFolders;
		};

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
			UI::Text pathText(m_currentFolder.string().substr(ProjectManager::CurrentProject().rootPath.parent_path().string().size()));

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
						UI::Icon icon(entry.path().filename().string(), EditorAssets::FileIcon(), { itemWidth ,itemWidth });
						// TODO : click : Tell inspector
						// TODO : double click : do something based on the file type

						// Asset drag and drop
						auto type = RexEngine::AssetTypes::GetAssetTypeFromExtension(entry.path().extension().string());
						if (!type.Empty())
						{ // This is an asset
							UI::DragDrop::Source<std::filesystem::path>("Asset" + type.name,
								entry.path(),
								entry.path().filename().string());
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
		}

	private:

		void OnProjectLoad()
		{
			m_currentFolder = ProjectManager::CurrentProject().rootPath;
		}

	private:
		std::filesystem::path m_currentFolder;
		float m_scale = 1.5f;
	};
}