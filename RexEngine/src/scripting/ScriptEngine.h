#pragma once

#include <memory>

#include "ScriptHost.h"

#include "../scene/Entity.h"

#include "../events/EngineEvents.h"
#include "../utils/StaticConstructor.h"

namespace RexEngine
{
	class ScriptHost;

	// A helper class for the ScriptHost of the ScriptEngine c# project
	class ScriptEngine
	{
	public:
	
		static bool LoadAssembly(const std::string& name);

		// Reload all the assemblies in the engine
		static void ReloadEngine();

		// Somehow works even if the function is not __stdcall ?
		static bool RegisterInternalCall(const std::string& callName, void* func);

		// Get a managed static function in one of the loaded assemblies
		template<typename R, typename... Args>
		static ScriptFunc<R, Args...> GetManagedFunction(const std::string& typeName, const std::string& funcName)
		{
			return ScriptFunc<R, Args...>((R(__stdcall*)(Args...))m_getManagedCall(typeName.c_str(), funcName.c_str()));
		}

	private:
		static void RegisterApi();

		static void StartEngine();

		static void UpdateEngine();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStart().Register<&ScriptEngine::StartEngine>();
			EngineEvents::OnUpdate().Register<&ScriptEngine::UpdateEngine>();
		})

	private:

		inline static std::unique_ptr<ScriptHost> m_host = nullptr;
		inline static ScriptFunc<uint8_t, const char*, void*> m_setInternalCall = nullptr;
		inline static ScriptFunc<void*, const char*, const char*> m_getManagedCall = nullptr;

		inline static ScriptFunc<uint8_t, const char*> m_loadAssembly = nullptr;
		inline static ScriptFunc<void> m_unloadAssemblies = nullptr;

		inline static ScriptFunc<void, float> m_setDeltaTime = nullptr;
		inline static ScriptFunc<void, const char*, uint8_t, float> m_setActionData = nullptr;

		inline static std::vector<std::string> m_loadedAssemblies; // Used to reload all the assemblies
	};
}