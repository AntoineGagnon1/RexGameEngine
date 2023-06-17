#pragma once
#include "../core/EngineEvents.h"

#include <mono/utils/mono-forward.h>

#include <filesystem>

typedef struct _MonoAssembly MonoAssembly;

namespace RexEngine
{
	class MonoEngine
	{
	public:
		// Will return nullptr if it fails
		static MonoAssembly* LoadAssembly(const std::filesystem::path& path);

	private:
		static void Init();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStart().Register<&MonoEngine::Init>();
		})

	private:
		inline static MonoDomain* s_rootDomain;
		inline static MonoDomain* s_appDomain;
	};
}