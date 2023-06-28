#include <REDPch.h>

#include <core/EditorEvents.h>

namespace RexEditor
{
	class MonoProject
	{
		static void OnProjectLoad(Project project)
		{
			// TODO : select debug / release
			s_gameAssembly = Mono::Assembly::Load(project.rootPath / "bin" / "Debug" / (project.Name + ".dll"));
		}

		RE_STATIC_CONSTRUCTOR({
			EditorEvents::OnProjectLoadStart().Register<&MonoProject::OnProjectLoad>();
		})

	private:
		inline static std::unique_ptr<Mono::Assembly> s_gameAssembly;
	};

}