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

	void MonoApi::MonoStart()
	{
		RegisterLog();
		s_apiAssembly = Mono::Assembly::Load("mono/CSharpApi.dll");
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

	void MonoApi::OnAddClass([[maybe_unused]]const Mono::Assembly& assembly, Mono::Class class_)
	{
		const Mono::Assembly* apiAssembly = s_apiAssembly.get();
		if (!s_apiAssembly)
			apiAssembly = &assembly;

		auto scriptComponentClass = apiAssembly->GetClass("RexEngine", "ScriptComponent").value();
		if (class_.IsSubClassOf(scriptComponentClass))
		{
			s_scriptTypes.push_back(std::make_shared<ScriptType>(class_));
		}
	}

	void MonoApi::OnRemoveClass([[maybe_unused]]const Mono::Assembly& assembly, Mono::Class class_)
	{
		auto scene = Scene::CurrentScene();
		if (!scene)
			return;

		for (auto&& [e, c] : scene->GetComponents<ScriptComponent>())
		{
			c.RemoveScriptType(class_);
		}

		s_scriptTypes.erase(std::remove_if(s_scriptTypes.begin(), s_scriptTypes.end(), [&class_](std::shared_ptr<ScriptType> type) {return type->GetClass() == class_; }), s_scriptTypes.end());
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

