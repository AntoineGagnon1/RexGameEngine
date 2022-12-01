#include "REDPch.h"
#include "ProjectManager.h"

#include <fstream>

#include "ui/MenuBar.h"
#include "ui/SystemDialogs.h"
#include "ui/panels/PanelManager.h"
#include "ui/panels/NewProject.h"

namespace RexEditor
{
	bool ProjectManager::Create(const std::filesystem::path& path, const std::string& name)
	{
		if (!std::filesystem::exists(path))
			return false;

		auto rootPath = path / name;

		auto root = std::filesystem::create_directory(rootPath);
		if (!root)
			return false;

		Project p;
		p.Name = name;
		p.LastScene = "";
		p.rootPath = rootPath;

		std::ofstream file(rootPath / (name + Project::FileExtension));

		if (file.is_open())
		{
			file << "--RexEngine project file--" << std::endl;
			file << "Name=" << p.Name.substr(0, Project::MaxNameLength) << std::endl;
			file << "LastScene=" << p.LastScene.string().substr(0, Project::MaxPathLength) << std::endl;

			return true;
		}

		return false;
	}

	bool ProjectManager::Load(const std::filesystem::path& path)
	{
		std::ifstream file = std::ifstream(path);

		if (!file.is_open())
			return false;

		std::string line;
		Project	project;
		project.rootPath = path.parent_path();
		char nameBuff[Project::MaxNameLength + 1] = { '\0' }; // +1 for null terminator
		char lastSceneBuff[Project::MaxPathLength + 1] = { '\0' };
		while (std::getline(file, line))
		{
			if (sscanf(line.c_str(), "Name=%s", nameBuff)) { project.Name = nameBuff; }
			else if (sscanf(line.c_str(), "LastScene=%s", lastSceneBuff)) { project.LastScene = lastSceneBuff; }
		}

		if (project.Name.empty())
			return false; // Malformed
		
		s_currentProject = project;

		// Try to load the scene from the path, if a path was specified
		if (!project.LastScene.empty())
		{
			std::ifstream sceneFile(path.parent_path() / project.LastScene);
			s_currentScene = RexEngine::SceneManager::CreateScene();

			if (sceneFile.is_open())
				s_currentScene.DeserializeJson(sceneFile);
			else
				RE_LOG_WARN("Scene at {} not found !", project.LastScene.string()); // TODO : change this to editor log
		}

		// On project load event
		EditorEvents::OnLoadProject().Dispatch();

		return true;
	}


	void ProjectManager::Init()
	{
		// Register MenuBar functions
		MenuBar::RegisterMenuFunction("Project/New", [] { PanelManager::GetPanel<NewProjectPanel>()->Show(); });
		MenuBar::RegisterMenuFunction("Project/Open...", [] { 
			
			auto path = SystemDialogs::SelectFile("Select a project to open", {"RexEngine Project (.rexengine)", "*.rexengine"});

			if (!Load(path))
				SystemDialogs::Alert("Error while loading the project", "Could not open the project at : " + path.string());
		});
	}
}