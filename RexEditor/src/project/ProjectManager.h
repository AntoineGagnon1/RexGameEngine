#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

#include "core/EditorEvents.h"

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

		// This will save the project to the .rexengine file,
		// this only saves the properties of the Project class, not scenes
		// returns false if the save failed
		static bool SaveProject(const Project& project);

		inline static Project& CurrentProject() { return s_currentProject; }

	private:

		static void OpenScene(const std::filesystem::path& path);

		static void Init();

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnEditorStart().Register<&Init>();
		});

	private:

		inline static Project s_currentProject;
	};
}