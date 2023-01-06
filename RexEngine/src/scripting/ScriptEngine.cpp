#include "REPch.h"
#include "ScriptEngine.h"

#include <filesystem>

#include "ScriptHost.h"
#include <core/FileStructure.h>
#include <core/Time.h>
#include <inputs/Inputs.h>
#include <scene/Entity.h>
#include <scene/Components.h>

namespace RexEngine::Internal
{
	void Assert(bool condition, const char* msg)
	{
		RE_ASSERT(condition, "[CSharp] {}", msg);
	}

	// level : 0 = info, 1 = warning, 2 = Error
	void Log(uint8_t level, const char* msg, uint32_t line, const char* func, const char* file)
	{
		switch (level)
		{
		case 0:
			Log::LogEvent().Dispatch(Log::LogType::Info, std::string(msg), line, std::string(func), std::string(file));
			break;
		case 1:
			Log::LogEvent().Dispatch(Log::LogType::Warning, std::string(msg), line, std::string(func), std::string(file));
			break;
		default:
			Log::LogEvent().Dispatch(Log::LogType::Error, std::string(msg), line, std::string(func), std::string(file));
			break;
		}
	}

	void GuidToString(Guid guid, char* writeInto)
	{
		auto str = guid.ToString();
		strcpy(writeInto, str.c_str());
	}

	uint8_t IsEntityValid(Guid guid)
	{
		Entity e(guid);
		return e ? 1 : 0;
	}

	uint8_t HasComponent(Guid guid, int typeId)
	{
		Entity e(guid);

		switch (typeId)
		{
		case 0:
			return e.HasComponent<TransformComponent>();

		default:
			RE_ASSERT(false, "Component type {} no found !", typeId);
			return 0;
		}
	}
}

namespace RexEngine
{
	bool ScriptEngine::LoadAssembly(const std::filesystem::path& path)
	{
		m_loadedAssemblies.push_back(path.string());
		return (bool)m_loadAssembly(path.string().c_str());
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

		// Scene
		RE_ASSERT(RegisterInternalCall("RexEngine.SceneCalls.IsEntityValid", (void*)Internal::IsEntityValid),
			"Error registering RexEngine.SceneCalls.IsEntityValid !");
		RE_ASSERT(RegisterInternalCall("RexEngine.SceneCalls.HasComponent", (void*)Internal::HasComponent),
			"Error registering RexEngine.SceneCalls.HasComponent !");

		// Get Managed functions in ScriptApi
		m_setDeltaTime = GetManagedFunction<void, float>("RexEngine.Time", "SetDeltaTime");
		RE_ASSERT(m_setDeltaTime != nullptr, "Error loading the RexEngine.Time.SetDeltaTime function !");

		m_setActionData = GetManagedFunction<void, const char*, uint8_t, float>("RexEngine.Inputs", "SetActionData");
		RE_ASSERT(m_setActionData != nullptr, "Error loading the RexEngine.Inputs.SetActionData function !");
	}

	void ScriptEngine::StartEngine()
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
		LoadAssembly(Dirs::ScriptEngineDir / "ScriptApi.dll");

		// Register the internal calls
		RegisterApi();
	}

	void ScriptEngine::UpdateEngine()
	{
		m_setDeltaTime(Time::DeltaTime());

		for (auto&& name : Inputs::GetActions())
		{
			auto& action = Inputs::GetAction(name);

			uint8_t bools = 0;
			bools |= action.IsDown();
			bools |= action.IsJustDown() << 1;
			bools |= action.IsJustUp() << 2;

			m_setActionData(name.c_str(), bools, action.GetValue());
		}
	}
}