#pragma once

#include <string>
#include <filesystem>

namespace RexEditor
{
	struct Project
	{
	private:
		friend class ProjectManager;
		Project() {}

	public:

		std::string Name;
		std::filesystem::path LastScene; // Relative path to the last scene opened in the editor, this is the scene 
										 // that will be opened when loading the project

		std::filesystem::path rootPath; // Path to the root of this project (location of the .rexengine file)

		inline static const std::string FileExtension = ".rexengine";
		static constexpr int MaxNameLength = 50; // Max project name length
		static constexpr int MaxPathLength = 250; // Max length of a path
	};
}