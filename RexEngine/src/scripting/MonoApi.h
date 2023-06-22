#pragma once
#include "Mono.h"
#include "../scene/ComponentFactory.h"


namespace RexEngine
{
	class Script;

	class ScriptType
	{
	public:
		ScriptType(const Mono::Class& class_);

		const Mono::Class& GetClass() const { return m_class; }

		void CallOnUpdate(const Script& script) const;

	private:
		Mono::Class m_class;
		std::function<void(MonoObject*)> m_onUpdate;
	};


	class MonoApi
	{
	public:

		// Returns all the script types registered
		static const auto& ScriptTypes() { return s_scriptTypes; }

		static const Mono::Assembly& GetApiAssembly() { return s_apiAssembly; }

		static std::shared_ptr<ScriptType> GetScriptType(const Mono::Class& class_);

	private:
		static void LoadScriptTypes(const Mono::Assembly& assembly);

		static void MonoStart();
		static void OnUpdate();

		RE_STATIC_CONSTRUCTOR({
			Mono::OnMonoStart().Register<&MonoApi::MonoStart>();
			EngineEvents::OnUpdate().Register<&MonoApi::OnUpdate>();
		});
	private:
		static void RegisterLog();

	private:
		inline static Mono::Assembly s_apiAssembly;
		inline static std::vector<std::shared_ptr<ScriptType>> s_scriptTypes;
	};

	class Script : public Mono::Object
	{
	public:
		Script(const Script&) = default;
		Script& operator=(const Script&) = default;
		Script() : Mono::Object(nullptr, {}), m_type(nullptr) {}

		std::vector<Mono::Field> GetSerializedFields() const;

		void CallOnUpdate() const { m_type->CallOnUpdate(*this); }

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(CUSTOM_NAME(GetClass().Namespace(), "TypeNamespace"));
			archive(CUSTOM_NAME(GetClass().Name(), "TypeName"));
			for (auto& field : GetSerializedFields())
			{
				// TODO : use some kind of map instead of checking for each type
				auto type = field.Type();
				if(TrySaveField<bool>(field, type, archive)) {}
				else if(TrySaveField<int8_t>(field, type, archive)) {}
				else if(TrySaveField<int16_t>(field, type, archive)) {}
				else if(TrySaveField<int32_t>(field, type, archive)) {}
				else if(TrySaveField<int64_t>(field, type, archive)) {}
				else if(TrySaveField<uint8_t>(field, type, archive)) {}
				else if(TrySaveField<uint16_t>(field, type, archive)) {}
				else if(TrySaveField<uint32_t>(field, type, archive)) {}
				else if(TrySaveField<uint64_t>(field, type, archive)) {}
				else if(TrySaveField<float>(field, type, archive)) {}
				else if(TrySaveField<double>(field, type, archive)) {}
			}
		}

		template <class Archive>
		void load(Archive& archive)
		{
			std::string name, namespace_;
			archive(CUSTOM_NAME(namespace_, "TypeNamespace"));
			archive(CUSTOM_NAME(name, "TypeName"));
			auto class_ = MonoApi::GetApiAssembly().GetClass(namespace_, name);
			if (!class_.has_value())
			{
				RE_LOG_ERROR("Failed to deserialize script, class {}.{} was not found", namespace_, name);
				return;
			}
			
			m_type = MonoApi::GetScriptType(class_.value());
			m_class = class_.value();
			m_object = Mono::Object::Create(class_.value()).value().GetPtr();
			for (auto& field : GetSerializedFields())
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
		static Script Create(std::shared_ptr<ScriptType> type);

		friend class ::cereal::access;
		// Only for serialization

		Script(MonoObject* obj, std::shared_ptr<ScriptType> type)
			: Mono::Object(obj, type->GetClass()), m_type(type) {}

	private:
		std::shared_ptr<ScriptType> m_type;
	};

	struct ScriptComponent
	{
	public:

		Script AddScript(std::shared_ptr<ScriptType> type);
		void RemoveScript(const Script& script);
		const std::vector<Script>& Scripts() const { return m_scripts; }

		template <class Archive>
		void serialize(Archive& archive)
		{
			archive(CUSTOM_NAME(m_scripts, "Scripts"));
		}

	private:
		std::vector<Script> m_scripts;
	};
	RE_REGISTER_COMPONENT(ScriptComponent, "ScriptComponent")
}