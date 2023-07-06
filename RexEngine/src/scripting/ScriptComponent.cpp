#include <REPch.h>
#include "ScriptComponent.h"

#include "MonoApi.h"
#include "../scene/Scene.h"

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

	Script Script::Create(std::shared_ptr<ScriptType> type, Entity parent)
	{
		return Script(Mono::Object::Create(type->GetClass()).value().GetPtr(), type, parent);
	}

	void Script::UpdateParent() const
	{
		if (m_object == nullptr || !m_class.has_value())
			return;
		
		const std::optional<Mono::Method> method = m_class.value().TryGetMethod("SetParent", 1);
		CallMethod(method.value(), m_parent.GetGuid());
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
		m_scripts.push_back(Script::Create(type, Scene::CurrentScene()->GetComponentOwner(*this)));
		return m_scripts.back();
	}

	void ScriptComponent::RemoveScript(const Script& script)
	{
		m_scripts.erase(std::remove(m_scripts.begin(), m_scripts.end(), script), m_scripts.end());
	}

	size_t ScriptComponent::RemoveScriptType(const Mono::Class& class_)
	{
		const auto oldSize = m_scripts.size();
		m_scripts.erase(std::remove_if(m_scripts.begin(), m_scripts.end(), [&class_](const Script& script) {return script.GetClass() == class_; }), m_scripts.end());
		return oldSize - m_scripts.size();
	}

	std::optional<Script> ScriptComponent::GetScript(const std::string& typeName) const
	{
		for (auto& s : m_scripts)
		{
			if (s.GetClass().Name() == typeName)
				return s;
		}
		return {};
	}
}