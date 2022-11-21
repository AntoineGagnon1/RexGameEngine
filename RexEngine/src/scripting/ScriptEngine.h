#pragma once

#include <memory>

#include "ScriptHost.h"

namespace RexEngine
{
	class ScriptHost;

	// A helper class for the ScriptHost of the ScriptEngine c# project
	class ScriptEngine
	{
	public:
		
		// Start the script engine, returns false if an error occured
		static bool Start();

		static bool LoadAssembly(const std::string& name);

		// Reload all the assemblies in the engine
		static void ReloadEngine();

		// Should be called every frame after Time::StartNewFrame
		// TODO : move this to a Engine event
		static void StartNewFrame();

		static void __stdcall TestFuncCpp(int a, Guid guid)
		{
			Guid g(3, 5);
			RE_LOG_WARN("HereCPP {} {}", a, guid.ToString());
		}

		static void test()
		{
			ReloadEngine();
			bool b = RegisterInternalCall("RexEngine.Test.TestFuncCpp", (void*)TestFuncCpp);
			
			//auto getFunc = m_host->GetFunction<void(*)(), const char*, const char*>("ScriptEngine", "ScriptEngine.Internal", "GetManagedFunction");
			//auto a = getFunc("RexEngine.Test", "TestFunc");
			auto a = GetManagedFunction<void>("RexEngine.Test", "TestFunc");
			//auto a = m_host->GetFunction<void>("ScriptApi", "RexEngine.Test", "TestFunc");
			a();
		}

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

	private:

		inline static std::unique_ptr<ScriptHost> m_host = nullptr;
		inline static ScriptFunc<uint8_t, const char*, void*> m_setInternalCall = nullptr;
		inline static ScriptFunc<void*, const char*, const char*> m_getManagedCall = nullptr;

		inline static ScriptFunc<uint8_t, const char*> m_loadAssembly = nullptr;
		inline static ScriptFunc<void> m_unloadAssemblies = nullptr;

		inline static ScriptFunc<void, float> m_setDeltaTime = nullptr;

		inline static std::vector<std::string> m_loadedAssemblies; // Used to reload all the assemblies
	};
}