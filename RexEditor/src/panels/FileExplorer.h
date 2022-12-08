#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "Panel.h"
#include "project/ProjectManager.h"
#include "ui/Gui.h"

#include "core/EditorAssets.h"

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
			if (Imgui::Button("<-") && m_currentFolder != ProjectManager::CurrentProject().rootPath)
			{
				m_currentFolder = m_currentFolder.parent_path();
			}

			// Scale slider
			Imgui::SameLine();
			Imgui::Space();
			Imgui::SameLine();
			Imgui::SliderFloat("Scale :", 0.25f, 2.0f, m_scale, 64);

			// Path label, start the path at the root folder
			Imgui::SameLine();
			Imgui::Text(m_currentFolder.string().substr(ProjectManager::CurrentProject().rootPath.parent_path().string().size()));

			// Calculate the number of columns
			int itemWidth = 64 * m_scale;
			int cols = (int)floor((float)PanelSize().x / (float)(itemWidth + 8));
			cols = RexEngine::Scalar::Clamp(cols, 0, 64); // imgui needs a value between 0 and 64

			// The file names should be small
			Imgui::PushFontScale(FontScale::Small);

			if (Imgui::BeginTable("FileExplorerTable", cols, {8,8}))
			{
				for (auto& entry : std::filesystem::directory_iterator(m_currentFolder))
				{
					Imgui::TableNextElement();
					if (entry.is_directory())
					{ // Folder
						bool clicked = false;
						bool doubleClicked = false; // go into the folder
						Imgui::IconButton(entry.path().filename().string(), EditorAssets::FolderIcon(), {itemWidth ,itemWidth}, clicked, doubleClicked);
						
						if (doubleClicked)
							m_currentFolder = entry.path();
					}
					else // File
					{
						bool clicked = false; // TODO : Tell inspector
						bool doubleClicked = false; // TODO : maybe open visual studio if .cs file ?
						Imgui::IconButton(entry.path().filename().string(), EditorAssets::FileIcon(), { itemWidth ,itemWidth }, clicked, doubleClicked);
					}
				}
				Imgui::EndTable();
			}

			Imgui::PopFontScale();
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