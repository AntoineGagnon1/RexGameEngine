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
		p.LastScenePath = "";
		p.RegistryPath = rootPath / (name + Project::RegistryFileExtension);

		std::ofstream file(rootPath / (name + Project::FileExtension));

		if (!file.is_open())
			return false;

		JsonSerializer archive(file);
		archive(CUSTOM_NAME(p, "Project"));

		return AssetManager::CreateRegistry(p.RegistryPath);
	}

	bool ProjectManager::Load(const std::filesystem::path& path)
	{
		std::ifstream file = std::ifstream(path);

		if (!file.is_open())
			return false;

		Project	p;
		p.rootPath = path.parent_path();

		JsonDeserializer archive(file);
		archive(p);

		if (p.Name.empty() || p.RegistryPath.empty())
			return false; // Malformed
		
		// Load the registry
		if (!AssetManager::SetRegistry(p.RegistryPath))
			return false;

		s_currentProject = p;

		// Try to load the scene from the path, if a path was specified
		if (!p.LastScenePath.empty())
		{
			std::ifstream sceneFile(path.parent_path() / p.LastScenePath);
			s_currentScene = RexEngine::SceneManager::CreateScene();

			if (sceneFile.is_open())
				s_currentScene.DeserializeJson(sceneFile);
			else
				RE_LOG_WARN("Scene at {} not found !", p.LastScenePath.string()); // TODO : change this to editor log
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