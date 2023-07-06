#pragma once
#include "Mono.h"
#include "MonoApi.h"
#include "../core/Serialization.h"
#include "../scene/ComponentFactory.h"

#include <typeindex>

namespace RexEngine
{
	class Script;

	class ScriptType
	{
	public:
		ScriptType(const Mono::Class& class_);
		~ScriptType();
		ScriptType(const ScriptType&) = delete;
		ScriptType& operator=(const ScriptType&) = delete;
		ScriptType(ScriptType&&) = delete;
		ScriptType& operator=(const ScriptType&&) = delete;

		const Mono::Class& GetClass() const { return m_class; }

		void CallOnUpdate(const Script& script) const;
		void CallOnStart(const Script& script) const;
		void CallOnDestroy(const Script& script) const;

	private:
		void Reload();
	private:
		Mono::Class m_class;
		std::function<void(MonoObject*)> m_onUpdate;
		std::function<void(MonoObject*)> m_onStart;
		std::function<void(MonoObject*)> m_onDestroy;
	};

	class Script : public Mono::Object
	{
	public:
		Script(const Script&) = default;
		Script& operator=(const Script&) = default;
		Script() : Mono::Object(nullptr, {}), m_type(nullptr) {}

		std::vector<Mono::Field> GetSerializedFields() const;

		void CallOnUpdate() const { m_type->CallOnUpdate(*this); }
		void CallOnStart() const { m_type->CallOnStart(*this); }
		void CallOnDestroy() const { m_type->CallOnDestroy(*this); }

		auto Type() const { return m_type; }

		// Is this script a valid instance ?
		// If the script was delete or did not load correctly this will return false
		bool IsValid() const { return m_type != nullptr && m_object != nullptr; }

		template <class Archive>
		void save(Archive& archive) const
		{
			if (m_object == nullptr || m_type == nullptr)
				return;

			archive(CUSTOM_NAME(m_parent, "Parent"));
			archive(CUSTOM_NAME(GetClass().Namespace(), "TypeNamespace"));
			archive(CUSTOM_NAME(GetClass().Name(), "TypeName"));
			for (auto& field : GetSerializedFields())
			{
				// TODO : use some kind of map instead of checking for each type
				auto type = field.Type();
				if (TrySaveField<bool>(field, type, archive)) {}
				else if (TrySaveField<int8_t>(field, type, archive)) {}
				else if (TrySaveField<int16_t>(field, type, archive)) {}
				else if (TrySaveField<int32_t>(field, type, archive)) {}
				else if (TrySaveField<int64_t>(field, type, archive)) {}
				else if (TrySaveField<uint8_t>(field, type, archive)) {}
				else if (TrySaveField<uint16_t>(field, type, archive)) {}
				else if (TrySaveField<uint32_t>(field, type, archive)) {}
				else if (TrySaveField<uint64_t>(field, type, archive)) {}
				else if (TrySaveField<float>(field, type, archive)) {}
				else if (TrySaveField<double>(field, type, archive)) {}
			}
		}

		template <class Archive>
		void load(Archive& archive)
		{
			archive(CUSTOM_NAME(m_parent, "Parent"));
			std::string name, namespace_;
			archive(CUSTOM_NAME(namespace_, "TypeNamespace"));
			archive(CUSTOM_NAME(name, "TypeName"));
			auto class_ = Mono::Assembly::FindClass(namespace_, name);
			if (!class_.has_value())
			{
				m_type = nullptr;
				m_object = nullptr;
				RE_LOG_ERROR("Failed to deserialize script, class {}.{} was not found", namespace_, name);
				return;
			}

			m_type = MonoApi::GetScriptType(class_.value());
			m_class = class_.value();
			m_object = Mono::Object::Create(class_.value()).value().GetPtr();
			UpdateParent();
			for (auto& field : GetSerializedFields())
			{
				try
				{
					auto type = field.Type();
					if (TryLoadField<bool>(field, type, archive)) {}
					else if (TryLoadField<int8_t>(field, type, archive)) {}
					else if (TryLoadField<int16_t>(field, type, archive)) {}
					else if (TryLoadField<int32_t>(field, type, archive)) {}
					else if (TryLoadField<int64_t>(field, type, archive)) {}
					else if (TryLoadField<uint8_t>(field, type, archive)) {}
					else if (TryLoadField<uint16_t>(field, type, archive)) {}
					else if (TryLoadField<uint32_t>(field, type, archive)) {}
					else if (TryLoadField<uint64_t>(field, type, archive)) {}
					else if (TryLoadField<float>(field, type, archive)) {}
					else if (TryLoadField<double>(field, type, archive)) {}
				}
				catch ([[maybe_unused]]const std::exception& e)
				{

				}
			}
		}

	private:
		template<typename T, typename Archive>
		bool TrySaveField(const Mono::Field& field, std::type_index type, Archive& archive) const
		{
			if (type == typeid(T))
			{
				archive(CUSTOM_NAME(GetValue<T>(field), field.Name()));
				return true;
			}
			return false;
		}

		template<typename T, typename Archive>
		bool TryLoadField(const Mono::Field& field, std::type_index type, Archive& archive)
		{
			if (type == typeid(T))
			{
				T value;
				archive(CUSTOM_NAME(value, field.Name()));
				SetValue<T>(field, value);
				return true;
			}
			return false;
		}

	private:
		friend class ::RexEngine::ScriptType;
		friend struct ScriptComponent;
		static Script Create(std::shared_ptr<ScriptType> type, Entity parent);

		friend class ::cereal::access;
		// Only for serialization

		Script(MonoObject* obj, std::shared_ptr<ScriptType> type, Entity parent)
			: Mono::Object(obj, type->GetClass()), m_type(type), m_parent(parent) 
		{
			UpdateParent();
		}

		void UpdateParent() const;
	private:
		std::shared_ptr<ScriptType> m_type;
		Entity m_parent;
	};

	struct ScriptComponent
	{
	public:
		Script AddScript(std::shared_ptr<ScriptType> type);
		void RemoveScript(const Script& script);
		// Will remove all the scripts of this type
		// Returns the number of scripts removed
		size_t RemoveScriptType(const Mono::Class& class_);
		const std::vector<Script>& Scripts() const { return m_scripts; }

		std::optional<Script> GetScript(const std::string& typeName) const;

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(CUSTOM_NAME(m_scripts, "Scripts"));
		}

		template <class Archive>
		void load(Archive& archive)
		{
			m_scripts.clear();
			archive(CUSTOM_NAME(m_scripts, "Scripts"));

			// Remove the scripts that did not load correctly (type does not exist anymore)
			m_scripts.erase(std::remove_if(m_scripts.begin(), m_scripts.end(), [](const Script& script) { return !script.IsValid(); }), m_scripts.end());
		}

	private:
		std::vector<Script> m_scripts;
	};
	RE_REGISTER_COMPONENT(ScriptComponent, "ScriptComponent")
}