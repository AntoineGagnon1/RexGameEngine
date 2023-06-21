#pragma once
#include "../core/EngineEvents.h"

#include <mono/utils/mono-forward.h>

#include <filesystem>

typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoString MonoString;
typedef struct _MonoClassField MonoClassField;
typedef struct _MonoCustomAttrInfo MonoCustomAttrInfo;
typedef union _MonoError MonoError;
typedef struct _MonoException MonoException;

namespace RexEngine
{
	class MonoEngine
	{
	public:
		RE_DECL_EVENT(OnMonoStart)

	public:
		// path is the path of the dll
		// name is a name used to reference it later
		// Will return nullptr if it fails
		static MonoAssembly* LoadAssembly(const std::filesystem::path& path, const std::string& name);


		// Will return nullptr if the class is not found
		static MonoClass* GetClass(MonoAssembly* assembly, const std::string& namespaceName, const std::string& className);
		static MonoMethod* GetMethod(MonoClass* class_, const std::string& methodName, int numArgs);
		// Will not report an error if the method was not found
		static MonoMethod* TryGetMethod(MonoClass* class_, const std::string& methodName, int numArgs);
		template<typename... Args>
		static std::function<void(Args...)> GetMethodThunk(MonoMethod* method)
		{
			// TODO : only add __stdcall on windows
			auto thunk = (void(__stdcall*)(Args..., MonoException**))GetMethodThunkInternal(method);
			if (thunk == nullptr)
				return nullptr;
			return [thunk] (Args... args) {
				MonoException* ex = nullptr;
				thunk(args..., &ex);
				if (ex != nullptr)
				{
					RE_LOG_ERROR("Error while calling method thunk");
				}
			};
		}
		static std::string GetClassName(MonoClass* class_);
		static std::string GetClassNamespace(MonoClass* class_);
		static MonoClass* GetParent(MonoClass* class_);
		static std::vector<MonoClassField*> GetFields(MonoClass* class_);

		static std::string GetFieldName(MonoClassField* field);
		static std::type_index GetFieldType(MonoClassField* field);
		template<typename T>
		static T GetFieldValue(MonoObject* instance, MonoClassField* field)
		{
			T value;
			GetFieldValueInternal(instance, field, &value);
			return value;
		}
		template<typename T>
		static void SetFieldValue(MonoObject* instance, MonoClassField* field, T value)
		{
			SetFieldValueInternal(instance, field, &value);
		}

		static MonoClass* GetClass(MonoObject* obj);
		// Will return nullptr if the creation failed
		static MonoObject* CreateObject(MonoClass* class_);
		// Uses mono_runtime_invoke (slower, safer)
		// the optional will be set if the method call worked
		template<typename... Args>
		static std::optional<MonoObject*> CallMethod(MonoMethod* method, void* thisPtr, Args... args)
		{
			static constexpr auto getValue = [](auto&& val) {
				if constexpr (std::is_same_v<decltype(val), MonoObject*>)
					return (void*)val;
				else
					return (void*)&val;
			};

			void* params[sizeof...(Args)];
			size_t i = 0;
			(void(params[i++] = getValue(args)), ...);
			
			return CallMethodInternal(method, thisPtr, params);
		}
		static std::optional<MonoObject*> CallMethod(MonoMethod* method, void* thisPtr)
		{
			return CallMethodInternal(method, thisPtr, nullptr);
		}


		// Name must be the full path, like : RexEngine.SomeClass::SomeMethod
		static void RegisterCall(const std::string& name, const void* function);


		static MonoAssembly* GetMainAssembly() { return s_mainAssembly; }
		static MonoAssembly* GetAssembly(const std::string& name);
		static std::string_view GetAssemblyName(MonoAssembly* assembly);
		static std::vector<MonoAssembly*> GetAssemblies();
		static std::vector<MonoClass*> GetTypes(MonoAssembly* assembly);

		static MonoCustomAttrInfo* GetAttributes(MonoClass* class_, MonoClassField* field);
		static bool ContainsAttribute(MonoCustomAttrInfo* attributes, MonoClass* attribute);

		static std::string GetString(MonoString* string);
		template<typename T>
		static T Unbox(MonoObject* obj)
		{
			return *(T*)UnboxInternal(obj);
		}
	private:
		static std::optional<MonoObject*> CallMethodInternal(MonoMethod* method, void* thisPtr, void* params[]);

		static std::string GetExceptionMessage(MonoObject* exception);
		
		static bool CheckMonoError(MonoError& error);

		static void* UnboxInternal(MonoObject* obj);

		static void GetFieldValueInternal(MonoObject* instance, MonoClassField* field, void* value);
		static void SetFieldValueInternal(MonoObject* instance, MonoClassField* field, void* value);

		static void* GetMethodThunkInternal(MonoMethod* method);

		static void Init();
		static void Stop();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStart().Register<&MonoEngine::Init>();
			EngineEvents::OnEngineStop().Register<&MonoEngine::Stop>();
		})

	private:
		inline static MonoDomain* s_rootDomain;
		inline static MonoDomain* s_appDomain;
		inline static MonoAssembly* s_mainAssembly;

		inline static std::unordered_map<std::string, MonoAssembly*> s_loadedAssemblies;
	};
}