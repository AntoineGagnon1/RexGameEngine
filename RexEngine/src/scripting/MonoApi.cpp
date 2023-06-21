#include <REPch.h>
#include "MonoApi.h"

#include <scene/Scene.h>

namespace RexEngine
{
	const Script& ScriptComponent::AddScript(std::shared_ptr<ScriptType> type)
	{
		m_scripts.push_back(Script(MonoEngine::CreateObject(type->GetClass()), type));
		return m_scripts.back();
	}

	void ScriptComponent::RemoveScript(const Script& script)
	{
		m_scripts.erase(std::remove(m_scripts.begin(), m_scripts.end(), script), m_scripts.end());
	}

	std::vector<MonoClassField*> MonoApi::GetSerializedFields(MonoClass* class_)
	{
		// For each field : check if it has the ShowInEditor attribute, if not, remove it
		static auto showInEditorClass = MonoEngine::GetClass(s_apiAssembly, "RexEngine", "ShowInEditorAttribute");
		auto fields = MonoEngine::GetFields(class_);
		fields.erase(
			std::remove_if(fields.begin(), fields.end(), [class_](MonoClassField* field) {
				if(MonoEngine::ContainsAttribute(MonoEngine::GetAttributes(class_, field), showInEditorClass))
					return false; // Keep this field
				return true;
			}),
			fields.end());
		return fields;
	}

	std::shared_ptr<ScriptType> MonoApi::GetScriptType(MonoClass* class_)
	{
		for (auto& type : s_scriptTypes)
		{
			if (type->GetClass() == class_)
				return type;
		}

		return nullptr;
	}

	void MonoApi::LoadScriptTypes(MonoAssembly* assembly)
	{
		auto scriptComponentClass = MonoEngine::GetClass(s_apiAssembly, "RexEngine", "ScriptComponent");
		auto scriptTypes = MonoEngine::GetTypes(assembly);
		for (auto type : scriptTypes)
		{
			MonoClass* parent = type;
			do
			{
				parent = MonoEngine::GetParent(parent);
				if (parent == scriptComponentClass)
				{
					s_scriptTypes.push_back(std::make_shared<ScriptType>(type));
					break;
				}
			} while (parent != nullptr);
		}
	}

	void MonoApi::MonoStart()
	{
		s_apiAssembly = MonoEngine::LoadAssembly("mono/CSharpApi.dll", "EngineApi");
		LoadScriptTypes(s_apiAssembly);
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
		Log::DispatchLog(LogType, line, MonoEngine::GetString(funcName), MonoEngine::GetString(fileName), MonoEngine::GetString(message));
	}

	void MonoApi::RegisterLog()
	{
		MonoEngine::RegisterCall("RexEngine.Log::Info", LogMessage<Log::LogType::Info>);
		MonoEngine::RegisterCall("RexEngine.Log::Warning", LogMessage<Log::LogType::Warning>);
		MonoEngine::RegisterCall("RexEngine.Log::Error", LogMessage<Log::LogType::Error>);
		MonoEngine::RegisterCall("RexEngine.Log::Debug", LogMessage<Log::LogType::Debug>);
	}

	ScriptType::ScriptType(MonoClass* class_)
		: m_class(class_)
	{
		auto updateMethod = MonoEngine::TryGetMethod(m_class, "OnUpdate", 0);
		if(updateMethod != nullptr)
			m_onUpdate = MonoEngine::GetMethodThunk<MonoObject*>(updateMethod);
	}

	void ScriptType::CallOnUpdate(const Script& script)
	{
		if (!m_onUpdate)
			return; // No OnUpdate function defined

		m_onUpdate(script.GetObject());
	}
}

