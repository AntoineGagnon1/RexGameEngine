#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../EditorEvents.h"

namespace RexEditor
{
	class SystemDialogs
	{
	public:
		// This will poll events while waiting for an action from the user
		static std::filesystem::path SelectFolder(const std::string& prompt = "Select a folder");

		// This will poll events while waiting for an action from the user
		static std::vector<std::filesystem::path> SelectFiles(const std::string& prompt, const std::vector<std::string>& filters, bool multiselect = true);

		// This will poll events while waiting for an action from the user
		inline static std::filesystem::path SelectFile(const std::string& prompt, const std::vector<std::string>& filters)
		{
			auto result = SelectFiles(prompt, filters, false);
			return result.empty() ? "" : result.front();
		}

		// This will poll events while waiting for an action from the user
		static std::filesystem::path SaveFile(const std::string& prompt, const std::vector<std::string>& filters);

		// An alert with a ok button, wont wait for user input
		// This will poll events while waiting for an action from the user
		static void Alert(const std::string& title, const std::string& message);

    private:

        static void Init();

        RE_STATIC_CONSTRUCTOR({
            EditorEvents::OnEditorStart().Register<&Init>();
        });

	};
}