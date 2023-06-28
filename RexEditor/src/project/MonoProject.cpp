#include <REDPch.h>

#include <core/EditorEvents.h>
#include <ui/MenuBar.h>

#include <efsw/efsw.hpp>

namespace RexEditor
{
    class AssemblyListener : public efsw::FileWatchListener 
    {
    public:
        void handleFileAction([[maybe_unused]]efsw::WatchID watchid, [[maybe_unused]]const std::string& dir,
            const std::string& filename, efsw::Action action, std::string oldFilename) override 
        {
            if (action == efsw::Action::Modified && filename.contains(".dll"))
			{
				ShouldReloadAssemblies = true;
            }
        }

		std::atomic_bool ShouldReloadAssemblies = false;
    };

	class MonoProject
	{
		static void OnProjectLoad(Project project)
		{
			// TODO : select debug / release
			const auto gameDLLDir = project.rootPath / "bin" / "Debug";
			s_gameAssembly = Mono::Assembly::Load(gameDLLDir / (project.Name + ".dll"));

			s_gameDLLWatcher.addWatch(gameDLLDir.string(), &s_dllChangedListener, true);
			s_gameDLLWatcher.watch();
		}

		static void OnEditorUpdate([[maybe_unused]]float dt)
		{
			if (s_dllChangedListener.ShouldReloadAssemblies.exchange(false))
			{
				Mono::ReloadAssemblies(true);
				RE_LOG_INFO("Reloaded the C# assemblies");
			}
		}

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnProjectLoadStart().Register<&MonoProject::OnProjectLoad>();
			EditorEvents::OnUI().Register<&MonoProject::OnEditorUpdate>();

			UI::MenuBar::RegisterMenuFunction("Scripting/Reload Assemblies", [] { Mono::ReloadAssemblies(true); });
		})

	private:
		inline static std::unique_ptr<Mono::Assembly> s_gameAssembly;
		inline static AssemblyListener s_dllChangedListener;
		inline static efsw::FileWatcher s_gameDLLWatcher;
	};

}