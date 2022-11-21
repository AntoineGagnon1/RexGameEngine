#include "REPch.h"
#include "ScriptEngine.h"

#include "ScriptHost.h"
#include <core/FileStructure.h>
#include <core/Time.h>

namespace RexEngine::Internal
{
	void Assert(bool condition, const char* msg)
	{
		RE_ASSERT(condition, "[CSharp] {}", msg);
	}

	// level : 0 = info, 1 = warning, 2 = Error
	void Log(const char* callerInfo, const char* msg, uint8_t level)
	{
		switch (level)
		{
		case 0:
			Log::Print(fg(fmt::color::gray), "CSHARP-INFO", "{} - {}", callerInfo, msg);
			break;
		case 1:
			Log::Print(fg(fmt::color::yellow), "CSHARP-WARNING", "{} - {}", callerInfo, msg);
			break;
		default:
			Log::Print(fg(fmt::color::red) | fmt::emphasis::bold, "CSHARP-ERROR", "{} - {}", callerInfo, msg);
			break;
		}
	}

	void GuidToString(Guid guid, char* writeInto)
	{
		auto str = guid.ToString();
		strcpy(writeInto, str.c_str());
	}
}

namespace RexEngine
{
	bool ScriptEngine::Start()
	{
		m_host = std::make_unique<ScriptHost>(Dirs::ScriptEngineDir, "ScriptEngine");
		RE_ASSERT(m_host != nullptr, "Error loading the ScriptEngine !");

		// Load the functions
		m_setInternalCall = m_host->GetFunction<uint8_t, const char*, void*>("ScriptEngine", "ScriptEngine.Internal", "SetInternalCall");
		m_getManagedCall = m_host->GetFunction<void*, const char*, const char*>("ScriptEngine", "ScriptEngine.Internal", "GetManagedFunction");
		m_loadAssembly = m_host->GetFunction<uint8_t, const char*>("ScriptEngine", "ScriptEngine.Internal", "LoadAssembly");
		m_unloadAssemblies = m_host->GetFunction<void>("ScriptEngine", "ScriptEngine.Internal", "UnloadAssemblies");

		RE_ASSERT(m_unloadAssemblies != nullptr, "Error loading the ScriptEngine.UnloadAssemblies function !");
		RE_ASSERT(m_loadAssembly != nullptr, "Error loading the ScriptEngine.LoadAssembly function !");
		RE_ASSERT(m_getManagedCall != nullptr, "Error loading the ScriptEngine.GetManagedFunction function !");
		RE_ASSERT(m_setInternalCall != nullptr, "Error loading the ScriptEngine.SetInternalCall function !");

		// Load the ScriptApi Assembly
		LoadAssembly("Dotnet/ScriptEngine/ScriptApi.dll");

		// Register the internal calls
		RegisterApi();

		return true;
	}

	bool ScriptEngine::LoadAssembly(const std::string& name)
	{
		m_loadedAssemblies.push_back(name);
		return (bool)m_loadAssembly(name.c_str());
	}

	void ScriptEngine::ReloadEngine()
	{
		// Unload the assemblies
		m_unloadAssemblies();

		// Save the loaded assemblies
		auto assemblies = std::move(m_loadedAssemblies);
		m_loadedAssemblies = std::vector<std::string>();

		// Reload all the assemblies that where loaded
		for (auto& a : assemblies)
			LoadAssembly(a);

		RegisterApi(); // The internal calls where lost, set them again
	}

	void ScriptEngine::StartNewFrame()
	{
		if (m_setDeltaTime != nullptr)
		{
			m_setDeltaTime(Time::DeltaTime());
		}
	}

	bool ScriptEngine::RegisterInternalCall(const std::string& callName, void* func)
	{
		return (bool)m_setInternalCall(callName.c_str(), (void*)func);
	}

	void ScriptEngine::RegisterApi()
	{
		// Register internal calls
		
		// Assert
		RE_ASSERT(RegisterInternalCall("RexEngine.CoreCalls.Assert", (void*)Internal::Assert),
			"Error registering RexEngine.CoreCalls.Assert !");

		// Log
		RE_ASSERT(RegisterInternalCall("RexEngine.CoreCalls.Log", (void*)Internal::Log),
			"Error registering RexEngine.CoreCalls.Log !");

		// Guid
		RE_ASSERT(RegisterInternalCall("RexEngine.CoreCalls.GenGuid", (void*)Guid::Generate),
			"Error registering RexEngine.CoreCalls.GenGuid !");
		RE_ASSERT(RegisterInternalCall("RexEngine.CoreCalls.GuidToString", (void*)Internal::GuidToString),
			"Error registering RexEngine.CoreCalls.GuidToString !");

		// Get Managed functions in ScriptApi
		m_setDeltaTime = GetManagedFunction<void, float>("RexEngine.Time", "SetDeltaTime");
		RE_ASSERT(m_setDeltaTime != nullptr, "Error loading the RexEngine.Time.SetDeltaTime function !");
	}
}