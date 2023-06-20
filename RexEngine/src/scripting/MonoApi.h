#pragma once
#include "MonoEngine.h"
#include "../scene/ComponentFactory.h"

namespace RexEngine
{
	class MonoApi
	{
	public:

		// Returns all the script types registered
		static std::vector<MonoClass*> GetScriptTypes();

		static std::vector<MonoClassField*> GetSerializedFields(MonoClass* class_);

		static MonoAssembly* GetApiAssembly() { return s_apiAssembly; }

	private:
		static void MonoStart();

		RE_STATIC_CONSTRUCTOR({
			MonoEngine::OnMonoStart().Register<&MonoApi::MonoStart>();
		});
	private:
		static void RegisterLog();

	private:
		inline static MonoAssembly* s_apiAssembly = nullptr;
	};

	class Script
	{
	public:
		Script() : Script(nullptr) {}

		Script(MonoObject* obj)
			: m_object(obj) {}

		std::string GetClassName() const { return MonoEngine::GetClassName(GetClass()); }
		std::string GetClassNamespace() const { return MonoEngine::GetClassNamespace(GetClass()); }
		auto GetSerializedFields() const { return MonoApi::GetSerializedFields(GetClass()); }

		std::type_index GetFieldType(MonoClassField* field) const { return MonoEngine::GetFieldType(field); }
		template<typename T>
		T GetFieldValue(MonoClassField* field) const { return MonoEngine::GetFieldValue<T>(m_object, field); }
		template<typename T>
		void SetFieldValue(MonoClassField* field, T value) const { return MonoEngine::SetFieldValue<T>(m_object, field, value); }
		std::string GetFieldName(MonoClassField* field) const { return MonoEngine::GetFieldName(field); }

		auto GetObject() const { return m_object; }

		auto operator<=>(const Script&) const = default;

		template <class Archive>
		void save(Archive& archive) const
		{
			archive(CUSTOM_NAME(GetClassNamespace(), "TypeNamespace"));
			archive(CUSTOM_NAME(GetClassName(), "TypeName"));
			for (auto field : GetSerializedFields())
			{
				// TODO : use some kind of map instead of checking for each type
				auto type = MonoEngine::GetFieldType(field);
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
			auto class_ = MonoEngine::GetClass(MonoApi::GetApiAssembly(), namespace_, name);
			if (class_ == nullptr)
			{
				RE_LOG_ERROR("Failed to deserialize script, class {}.{} was not found", namespace_, name);
				return;
			}
			
			m_object = MonoEngine::CreateObject(class_);
			for (auto field : GetSerializedFields())
			{
				auto type = MonoEngine::GetFieldType(field);
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
		bool TrySaveField(MonoClassField* field, std::type_index type, Archive& archive) const
		{
			if (type == typeid(T))
			{
				archive(CUSTOM_NAME(GetFieldValue<T>(field), GetFieldName(field)));
				return true;
			}
			return false;
		}

		template<typename T, typename Archive>
		bool TryLoadField(MonoClassField* field, std::type_index type, Archive& archive)
		{
			if (type == typeid(T))
			{
				T value;
				archive(CUSTOM_NAME(value, GetFieldName(field))); 
				SetFieldValue<T>(field, value);
				return true;
			}
			return false;
		}

	private:
		MonoClass* GetClass() const { return MonoEngine::GetClass(m_object); }

	private:
		MonoObject* m_object;
	};

	struct ScriptComponent
	{
	public:

		const Script& AddScript(MonoClass* class_);
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