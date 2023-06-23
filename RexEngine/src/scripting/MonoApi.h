#pragma once
#include "Mono.h"
#include "../scene/ComponentFactory.h"


namespace RexEngine
{
	class ScriptType;

	class MonoApi
	{
	public:

		// Returns all the script types registered
		static const auto& ScriptTypes() { return s_scriptTypes; }

		static const Mono::Assembly& GetApiAssembly() { return *s_apiAssembly; }

		static std::shared_ptr<ScriptType> GetScriptType(const Mono::Class& class_);

	private:
		static void LoadScriptTypes(const Mono::Assembly& assembly);

		static void MonoStart();
		static void OnUpdate();

		RE_STATIC_CONSTRUCTOR({
			Mono::OnMonoStart().Register<&MonoApi::MonoStart>();
			EngineEvents::OnUpdate().Register<&MonoApi::OnUpdate>();
		});
	private:
		static void RegisterLog();

	private:
		inline static std::unique_ptr<Mono::Assembly> s_apiAssembly;
		inline static std::vector<std::shared_ptr<ScriptType>> s_scriptTypes;
	};
}