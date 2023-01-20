#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "Panel.h"
#include "PanelManager.h"
#include "project/Project.h"
#include "project/ProjectManager.h"
#include "ui/SystemDialogs.h"
#include "ui/UIElements.h"

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
		virtual void OnGui([[maybe_unused]] float deltaTime) override
		{
			UI::TextInput nameInput("Project name : ", Project::MaxNameLength, m_name);

			if (UI::Button b("Select root path ..."); b.IsClicked())
			{
				auto path = SystemDialogs::SelectFolder("Select a folder to create the project into");
				m_path = path;
			}

			UI::SameLine();
			UI::Text pathText((m_path / m_name).string());

			UI::Anchor a(UI::AnchorPos::BottomRight);
			if (UI::Button b("Create"); b.IsClicked())
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