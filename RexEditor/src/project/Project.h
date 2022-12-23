#pragma once

#include <string>
#include <filesystem>

#include <RexEngine.h>

namespace RexEditor
{
	struct Project
	{
	private:
		friend class ProjectManager;
		Project() {}

	public:

		std::string Name;
		std::filesystem::path LastScenePath; // RELATIVE path to the last scene opened in the editor, this is the scene 
										 // that will be opened when loading the project (relative to the .rexengine file)


		// Not serialized
		std::filesystem::path rootPath; // Path to the root of this project (directory of the .rexengine file)

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(KEEP_NAME(Name), KEEP_NAME(LastScenePath));
		}

		inline static const std::string FileExtension = ".rexengine";
		static constexpr int MaxNameLength = 50; // Max project name length
		static constexpr int MaxPathLength = 250; // Max length of a path
	};
}