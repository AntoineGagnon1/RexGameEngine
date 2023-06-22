#include <REPch.h>
#include "MonoApi.h"

#include <scene/Scene.h>

namespace RexEngine
{
	Script ScriptComponent::AddScript(std::shared_ptr<ScriptType> type)
	{
		m_scripts.push_back(Script::Create(type));
		return m_scripts.back();
	}

	void ScriptComponent::RemoveScript(const Script& script)
	{
		m_scripts.erase(std::remove(m_scripts.begin(), m_scripts.end(), script), m_scripts.end());
	}

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
		auto scriptComponentClass = assembly.GetClass("RexEngine", "ScriptComponent");
		auto scriptTypes = assembly.GetTypes();
		for (auto& type : scriptTypes)
		{
			std::optional<Mono::Class> parent = type;
			do
			{
				parent = parent.value().Parent();
				if (parent == scriptComponentClass)
				{
					s_scriptTypes.push_back(std::make_shared<ScriptType>(type));
					break;
				}
			} while (parent.has_value());
		}
	}

	void MonoApi::MonoStart()
	{
		s_apiAssembly = Mono::LoadAssembly("mono/CSharpApi.dll").value();
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
		Log::DispatchLog(LogType, line, Mono::GetString(funcName), Mono::GetString(fileName), Mono::GetString(message));
	}

	void MonoApi::RegisterLog()
	{
		Mono::RegisterCall("RexEngine.Log::Info", LogMessage<Log::LogType::Info>);
		Mono::RegisterCall("RexEngine.Log::Warning", LogMessage<Log::LogType::Warning>);
		Mono::RegisterCall("RexEngine.Log::Error", LogMessage<Log::LogType::Error>);
		Mono::RegisterCall("RexEngine.Log::Debug", LogMessage<Log::LogType::Debug>);
	}

	ScriptType::ScriptType(const Mono::Class& class_)
		: m_class(class_)
	{
		auto onUpdate = m_class.TryGetMethod("OnUpdate", 0);
		if(onUpdate.has_value())
			m_onUpdate = onUpdate.value().GetThunk<MonoObject*>();
	}

	void ScriptType::CallOnUpdate(const Script& script) const
	{
		if (!m_onUpdate)
			return; // No OnUpdate function defined

		m_onUpdate(script.GetPtr());
	}

	Script Script::Create(std::shared_ptr<ScriptType> type)
	{
		return Script(Mono::Object::Create(type->GetClass()).value().GetPtr(), type);
	}

	std::vector<Mono::Field> Script::GetSerializedFields() const
	{
		// For each field : check if it has the ShowInEditor attribute, if not, remove it
		static auto showInEditorClass = MonoApi::GetApiAssembly().GetClass("RexEngine", "ShowInEditorAttribute");
		auto fields = GetClass().Fields();
		fields.erase(
			std::remove_if(fields.begin(), fields.end(), [](const Mono::Field& field) {
					if (field.HasAttribute(showInEditorClass.value()))
						return false; // Keep this field
					return true;
				}),
			fields.end());
		return fields;
	}
}

