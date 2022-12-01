#include "REDPch.h"
#include "SystemDialogs.h"

#include <portable-file-dialogs/portable-file-dialogs.h>
#include <RexEngine.h>

namespace RexEditor
{
	std::filesystem::path SystemDialogs::SelectFolder(const std::string& prompt)
	{
        auto dir = pfd::select_folder(prompt);

        while (!dir.ready())
        {
            RexEngine::Inputs::PollInputs();
        }

        return dir.result();
	}

    std::vector<std::filesystem::path> SystemDialogs::SelectFiles(const std::string& prompt, const std::vector<std::string>& filters, bool multiselect)
    {
        auto f = pfd::open_file(prompt, "", filters, multiselect ? pfd::opt::multiselect : pfd::opt::none);

        while (!f.ready())
        {
            RexEngine::Inputs::PollInputs();
        }

        auto files = f.result();
        std::vector<std::filesystem::path> paths;
        paths.reserve(files.size());

        for (auto& file : files)
            paths.push_back(file);

        return paths;
    }

    std::filesystem::path SystemDialogs::SaveFile(const std::string& prompt, const std::vector<std::string>& filters)
    {
        auto f = pfd::save_file(prompt, "", filters);

        while (!f.ready())
        {
            RexEngine::Inputs::PollInputs();
        }

        return f.result();
    }

    void SystemDialogs::Alert(const std::string& title, const std::string& message)
    {
        auto m = pfd::message(title, message, pfd::choice::ok, pfd::icon::error);
        while (!m.ready())
        {
            RexEngine::Inputs::PollInputs();
        }
    }

	void SystemDialogs::Init()
	{
        RE_ASSERT(pfd::settings::available(), "File dialogs are not available on this platform !");

        pfd::settings::verbose(false);
	}
}