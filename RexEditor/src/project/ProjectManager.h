#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "EditorEvents.h"

#include "Project.h"

namespace RexEditor
{
	// Load and save .rexengine projects
	class ProjectManager
	{
	public:

		// Create a project in the directory
		// this will create a new directory [name] at the path
		// returns false if one of the files could not be created
		static bool Create(const std::filesystem::path& path, const std::string& name);

		// Load a project (and set it as active)
		// returns false if the file does not exist or is malformed
		static bool Load(const std::filesystem::path& path);

		// This might be an invalid scene if no current scene was set
		inline static RexEngine::Scene CurrentScene() { return s_currentScene; }

		inline static Project& CurrentProject() { return s_currentProject; }

	private:

		static void Init();

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnEditorStart().Register<&Init>();
		});

	private:

		inline static Project s_currentProject;
		inline static RexEngine::Scene s_currentScene;
	};
}