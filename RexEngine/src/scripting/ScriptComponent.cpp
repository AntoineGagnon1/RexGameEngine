#include <REPch.h>
#include "ScriptComponent.h"

#include "MonoApi.h"

namespace RexEngine
{
	ScriptType::ScriptType(const Mono::Class& class_)
		: m_class(class_)
	{
		Mono::OnReload().Register<&ScriptType::Reload>(this);

		Reload();
	}

	ScriptType::~ScriptType()
	{
		Mono::OnReload().UnRegister<&ScriptType::Reload>(this);
	}

	void ScriptType::CallOnUpdate(const Script& script) const
	{
		if (!m_onUpdate)
			return; // No OnUpdate function defined

		m_onUpdate(script.GetPtr());
	}

	void ScriptType::Reload()
	{
		auto onUpdate = m_class.TryGetMethod("OnUpdate", 0);
		if (onUpdate.has_value())
			m_onUpdate = onUpdate.value().GetThunk<MonoObject*>();
	}

	Script Script::Create(std::shared_ptr<ScriptType> type)
	{
		return Script(Mono::Object::Create(type->GetClass()).value().GetPtr(), type);
	}

	std::vector<Mono::Field> Script::GetSerializedFields() const
	{
		// For each field : check if it has the ShowInEditor attribute, if not, remove it
		auto showInEditorClass = MonoApi::GetApiAssembly().GetClass("RexEngine", "ShowInEditorAttribute").value();
		auto fields = GetClass().Fields();
		fields.erase(
			std::remove_if(fields.begin(), fields.end(), [&showInEditorClass](const Mono::Field& field) {
				if (field.HasAttribute(showInEditorClass))
				return false; // Keep this field
		return true;
				}),
			fields.end());
		return fields;
	}

	Script ScriptComponent::AddScript(std::shared_ptr<ScriptType> type)
	{
		m_scripts.push_back(Script::Create(type));
		return m_scripts.back();
	}

	void ScriptComponent::RemoveScript(const Script& script)
	{
		m_scripts.erase(std::remove(m_scripts.begin(), m_scripts.end(), script), m_scripts.end());
	}
}