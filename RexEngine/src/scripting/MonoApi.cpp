#include <REPch.h>
#include "MonoApi.h"


namespace RexEngine
{
	MonoObject* ScriptComponent::AddScript(MonoClass* class_)
	{
		RE_ASSERT(class_ != nullptr, "Cannot create a <null> script !");

		auto script = MonoEngine::CreateObject(class_);
		m_scripts.push_back(script);
		return script;
	}

	void ScriptComponent::RemoveScript(MonoObject* script)
	{
		m_scripts.erase(std::remove(m_scripts.begin(), m_scripts.end(), script), m_scripts.end());
	}



	std::vector<MonoClass*> MonoApi::GetScriptTypes()
	{
		auto scriptComponentClass = MonoEngine::GetClass(s_apiAssembly, "RexEngine", "ScriptComponent");
		std::vector<MonoClass*> types;
		auto assemblies = MonoEngine::GetAssemblies(); // TODO : Only check for engine and game assembly (no editor)
		for (auto assembly : assemblies)
		{
			auto scriptTypes = MonoEngine::GetTypes(assembly);
			for (auto type : scriptTypes)
			{
				MonoClass* parent = type;
				do
				{
					parent = MonoEngine::GetParent(parent);
					if (parent == scriptComponentClass)
					{
						types.push_back(type);
						break;
					}
				} while (parent != nullptr);
			}
		}

		return types;
	}

	void MonoApi::MonoStart()
	{
		s_apiAssembly = MonoEngine::LoadAssembly("mono/CSharpApi.dll", "EngineApi");
		RegisterLog();
	}

	template<Log::LogType LogType>
	static void LogMessage(MonoString* message, int line, MonoString* funcName, MonoString* fileName)
	{
		Log::DispatchLog(LogType, line, MonoEngine::GetString(funcName), MonoEngine::GetString(fileName), MonoEngine::GetString(message));
	}

	void MonoApi::RegisterLog()
	{
		MonoEngine::RegisterCall("RexEngine.Log::Info", LogMessage<Log::LogType::Info>);
		MonoEngine::RegisterCall("RexEngine.Log::Warning", LogMessage<Log::LogType::Warning>);
		MonoEngine::RegisterCall("RexEngine.Log::Error", LogMessage<Log::LogType::Error>);
		MonoEngine::RegisterCall("RexEngine.Log::Debug", LogMessage<Log::LogType::Debug>);
	}
}

