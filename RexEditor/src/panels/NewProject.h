#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "Panel.h"
#include "PanelManager.h"
#include "project/Project.h"
#include "project/ProjectManager.h"
#include "ui/Gui.h"
#include "ui/SystemDialogs.h"

namespace RexEditor
{
	class NewProjectPanel : public Panel
	{
	public:
		NewProjectPanel() : Panel("Create a new project")
		{
			CanDock(false);
			Hide();
		}

	protected:
		virtual void OnGui(float deltaTime) override
		{
			Imgui::TextInput("Project name : ", m_name, Project::MaxNameLength);

			if (Imgui::Button("Select root path ..."))
			{
				auto path = SystemDialogs::SelectFolder("Select a folder to create the project into");
				m_path = path;
			}

			Imgui::SameLine();
			Imgui::Text((m_path / m_name).string());

			if (Imgui::Button("Create", Alignement::Right, VerticalPos::Bottom))
			{
				if (!std::filesystem::exists(m_path))
					SystemDialogs::Alert("Error while creating the project", "Invalid path !");
				else
				{
					// Create the project
					if(!ProjectManager::Create(m_path, m_name))
						SystemDialogs::Alert("Error while creating the project", "Could not create the project files !");
					else
					{
						// Load the project
						if (!ProjectManager::Load(m_path / m_name / (m_name + Project::FileExtension)))
							SystemDialogs::Alert("Error while loading the project", "Could not load the new project !");
					}

					// Hide this panel
					Hide();
				}
			}
		}

	private:
		std::string m_name;
		std::filesystem::path m_path;
	};
}