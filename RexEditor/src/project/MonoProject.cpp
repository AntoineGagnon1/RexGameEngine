#include <REDPch.h>

namespace RexEditor
{
	class MonoProject
	{
		static void MonoStart()
		{

		}

		RE_STATIC_CONSTRUCTOR({
			Mono::OnMonoStart().Register<&MonoProject::MonoStart>();
		})
	};

}