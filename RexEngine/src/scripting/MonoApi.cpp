#include <REPch.h>
#include "MonoApi.h"

#include "ScriptComponent.h"

#include <scene/Scene.h>

namespace RexEngine
{
	std::shared_ptr<ScriptType> MonoApi::GetScriptType(const Mono::Class& class_)
	{
		for (auto& type : s_scriptTypes)
		{
			if (type->GetClass() == class_)
				return type;
		}

		return nullptr;
	}

	void MonoApi::LoadScriptTypes(const Mono::Assembly& assembly)
	{
		auto scriptComponentClass = assembly.GetClass("RexEngine", "ScriptComponent").value();
		auto scriptTypes = assembly.GetTypes();
		for (auto& type : scriptTypes)
		{
			if(type.IsSubClassOf(scriptComponentClass))
				s_scriptTypes.push_back(std::make_shared<ScriptType>(type));
		}
	}

	void MonoApi::MonoStart()
	{
		s_apiAssembly = Mono::Assembly::Load("mono/CSharpApi.dll");
		LoadScriptTypes(*s_apiAssembly);
		RegisterLog();
	}

	void MonoApi::OnUpdate()
	{
		auto scene = Scene::CurrentScene();
		if (!scene)
			return;

		auto scriptComponents = scene->GetComponents<ScriptComponent>();
		for (auto&& [entity, scriptComponent] : scriptComponents)
		{
			for (auto& script : scriptComponent.Scripts())
			{
				script.CallOnUpdate();
			}
		}
	}

	template<Log::LogType LogType>
	static void LogMessage(MonoString* message, int line, MonoString* funcName, MonoString* fileName)
	{
		Log::DispatchLog(LogType, line, Mono::GetString(funcName), Mono::GetString(fileName), Mono::GetString(message));
	}

	void MonoApi::RegisterLog()
	{
		Mono::RegisterCall("RexEngine.Log::Info", LogMessage<Log::LogType::Info>);
		Mono::RegisterCall("RexEngine.Log::Warning", LogMessage<Log::LogType::Warning>);
		Mono::RegisterCall("RexEngine.Log::Error", LogMessage<Log::LogType::Error>);
		Mono::RegisterCall("RexEngine.Log::Debug", LogMessage<Log::LogType::Debug>);
	}
}

