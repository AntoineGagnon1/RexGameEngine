#include "REDPch.h"
#include "ProjectManager.h"

#include <fstream>

#include "ui/MenuBar.h"
#include "ui/SystemDialogs.h"
#include "panels/PanelManager.h"
#include "panels/NewProject.h"

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
		p.rootPath = rootPath;

		return SaveProject(p);
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

		if (p.Name.empty())
			return false; // Malformed
		
		// Load the registry
		AssetManager::LoadRegistry(p.rootPath);

		s_currentProject = p;

		// Try to load the scene from the path, if a path was specified
		OpenScene(path.parent_path() / p.LastScenePath);

		// On project load event
		EditorEvents::OnLoadProject().Dispatch(std::forward<Project>(p));

		return true;
	}

	bool ProjectManager::SaveProject(const Project& project)
	{
		std::ofstream file(project.rootPath / (project.Name + Project::FileExtension));

		if (!file.is_open())
			return false;

		JsonSerializer archive(file);
		archive(CUSTOM_NAME(project, "Project"));
		return true;
	}

	void ProjectManager::OpenScene(const std::filesystem::path& path)
	{
		auto scene = AssetManager::GetAsset<Scene>(AssetManager::GetAssetGuidFromPath(path));
		if (scene)
		{
			Scene::SetCurrentScene(scene);

			// Save as the last open scene
			s_currentProject.LastScenePath = std::filesystem::relative(path, s_currentProject.rootPath);
			SaveProject(s_currentProject);

			// Callback
			EditorEvents::OnOpenScene().Dispatch(std::forward<Asset<Scene>>(scene));
		}
		else
			RE_LOG_WARN("Scene at {} not found !", path.string());
	}

	void ProjectManager::Init()
	{
		// Register MenuBar functions
		UI::MenuBar::RegisterMenuFunction("Project/New...", [] { PanelManager::GetPanel<NewProjectPanel>()->Show(); });
		UI::MenuBar::RegisterMenuFunction("Project/Open...", [] {
			
			auto path = SystemDialogs::SelectFile("Select a project to open", {"RexEngine Project (.rexengine)", "*.rexengine"});

			if (!Load(path))
				SystemDialogs::Alert("Error while loading the project", "Could not open the project at : " + path.string());
		});

		// Scene
		UI::MenuBar::RegisterMenuFunction("Scene/Save", [] {
			auto scene = Scene::CurrentScene();
			AssetManager::SaveAsset<Scene>(scene.GetAssetGuid());
		});

		UI::MenuBar::RegisterMenuFunction("Scene/New...", [] {

			auto path = SystemDialogs::SaveFile("Select a location for the scene", { "RexEngine Scene (.scene)", "*.scene" });
			path = path.replace_extension(".scene");

			if (!path.empty())
			{
				auto scene = Scene::CreateScene();
				AssetManager::AddAsset<Scene>(scene->GetGuid(), path, Asset(scene->GetGuid(), scene));

				// Load the new scene
				OpenScene(path);
			}
		});

		UI::MenuBar::RegisterMenuFunction("Scene/Open...", [] {

			auto path = SystemDialogs::SelectFile("Select a scene to open", { "RexEngine Scene (.scene)", "*.scene" });

			if (!path.empty()) // Not canceled
				OpenScene(path);
		});

	}
}