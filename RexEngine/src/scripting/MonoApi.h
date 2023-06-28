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
		static void MonoStart();
		static void OnUpdate();

		static void OnAddClass(const Mono::Assembly& assembly, Mono::Class class_);
		static void OnRemoveClass(const Mono::Assembly& assembly, Mono::Class class_);

		RE_STATIC_CONSTRUCTOR({
			Mono::OnMonoStart().Register<&MonoApi::MonoStart>();
			Mono::OnAddClass().Register<&MonoApi::OnAddClass>();
			Mono::OnRemoveClass().Register<&MonoApi::OnRemoveClass>();
			EngineEvents::OnUpdate().Register<&MonoApi::OnUpdate>();
		});
	private:
		static void RegisterLog();

	private:
		inline static std::unique_ptr<Mono::Assembly> s_apiAssembly;
		inline static std::vector<std::shared_ptr<ScriptType>> s_scriptTypes;
	};
}