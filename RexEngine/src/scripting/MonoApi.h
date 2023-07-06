#pragma once
#include "Mono.h"
#include "../scene/ComponentFactory.h"
#include "../scene/Scene.h"

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

		static void OnSceneStart(Asset<Scene> scene);
		static void OnSceneStop(Asset<Scene> scene);

		RE_STATIC_CONSTRUCTOR({
			Mono::OnMonoStart().Register<&MonoApi::MonoStart>();
			Mono::OnAddClass().Register<&MonoApi::OnAddClass>();
			Mono::OnRemoveClass().Register<&MonoApi::OnRemoveClass>();
			EngineEvents::OnUpdate().Register<&MonoApi::OnUpdate>();
			Scene::OnSceneStart().Register<&MonoApi::OnSceneStart>();
			Scene::OnSceneStop().Register<&MonoApi::OnSceneStop>();
		});
	private:
		static void RegisterLog();
		static void RegisterGuid();
		static void RegisterScene();
		static void RegisterInputs();

	private:
		inline static std::unique_ptr<Mono::Assembly> s_apiAssembly;
		inline static std::vector<std::shared_ptr<ScriptType>> s_scriptTypes;
		inline static std::unordered_map<std::string, Mono::Class> s_componentClasses;
	};
}