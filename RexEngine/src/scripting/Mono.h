#pragma once
#include "../core/EngineEvents.h"

#include <mono/utils/mono-forward.h>

#include "../utils/TupleHash.h"

typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoString MonoString;
typedef struct _MonoClassField MonoClassField;
typedef union _MonoError MonoError;
typedef struct _MonoException MonoException;

namespace RexEngine
{
	class Mono
	{
	public:
		class Class;
		class Object;
		class Assembly;

		RE_DECL_EVENT(OnMonoStart)
		RE_DECL_EVENT(OnReload)

		// Called when a new c# class is loaded
		RE_DECL_EVENT(OnAddClass, const Assembly&, Class)
		// Called when a c# no longer exists
		RE_DECL_EVENT(OnRemoveClass, const Assembly&, Class)
	public:

		class Field
		{
		public:
			Field(MonoClassField* field) : m_field(field) {}
			Field(const Field&) = default;
			Field& operator=(const Field&) = default;

			std::string Name() const;
			// Will return typeid(void) if the type is not recognized
			std::type_index Type() const;

			bool HasAttribute(const Class& attributeType) const;

			auto GetPtr() const { return m_field; }
		private:
			MonoClassField* m_field;
		};



		class Method
		{
		public:
			Method(MonoMethod* method) : m_method(method) {}
			Method(const Method&) = default;
			Method& operator=(const Method&) = default;

			template<typename... Args>
			std::function<void(Args...)> GetThunk() const
			{
				// TODO : only add __stdcall on windows
				auto thunk = (void(__stdcall*)(Args..., MonoException**))GetThunkInternal();
				if (thunk == nullptr)
					return nullptr;
				return [thunk](Args... args) {
					MonoException* ex = nullptr;
					thunk(args..., &ex);
					if (ex != nullptr)
					{
						// TODO : extract msg from MonoException
						RE_LOG_ERROR("Error while calling method thunk");
					}
				};
			}

			auto GetPtr() const { return m_method; }
		private:
			void* GetThunkInternal() const;

		private:
			MonoMethod* m_method;
		};

		struct ClassProxy_
		{
			MonoClass* ptr;
		};

		class Class
		{
		public:
			Class(ClassProxy_* class_) : m_class(class_) {}
			Class(const Class&) = default;
			Class& operator=(const Class&) = default;

			std::optional<Method> TryGetMethod(const std::string& methodName, int numArgs) const;

			std::string Name() const;
			std::string Namespace() const;

			bool IsSubClassOf(const Class& parent);

			std::vector<Field> Fields() const;

			bool operator==(const Class& other) const { return m_class == other.m_class; }

			auto GetPtr() const { return m_class->ptr; }

		private:
			ClassProxy_* m_class;
		};



		class Assembly
		{
		public:
			~Assembly();
			Assembly(const Assembly&) = delete;
			Assembly& operator=(const Assembly&) = delete;

			static std::unique_ptr<Assembly> Load(const std::filesystem::path& dllPath);
			
			std::optional<Class> GetClass(const std::string& namespaceName, const std::string& className) const;
			
			std::string GetName() const;
			std::vector<Class> GetTypes() const;

			auto GetPtr() const { return m_assembly; }
			
			// Check in all loaded assemblies
			static std::optional<Class> FindClass(const std::string& namespaceName, const std::string& className);
		private:
			void Reload();
			static MonoAssembly* LoadAssemblyInternal(const std::filesystem::path& dllPath);
			Assembly(const std::filesystem::path& dllPath);


		private:
			MonoAssembly* m_assembly;
			std::filesystem::path m_dllPath;
			std::unordered_map<std::tuple<std::string, std::string>, std::unique_ptr<ClassProxy_>> m_classes;

			inline static std::vector<Assembly*> s_assemblies;
		};



		class Object
		{
		public:
			Object(MonoObject* obj, std::optional<Class> class_) : m_object(obj), m_class(class_) {}
			Object(const Object&) = default;
			Object& operator=(const Object&) = default;

			static std::optional<Object> Create(const Class& class_);

			const Class& GetClass() const { return m_class.value(); }

			template<typename T>
			T GetValue(const Field& field) const
			{
				T value;
				GetValueInternal(field, &value);
				return value;
			}
			template<typename T>
			void SetValue(const Field& field, T value) const { SetValueInternal(field, &value); }

			// Uses mono_runtime_invoke (slower, safer)
			// the optional will be set if the method call worked
			template<typename... Args>
			std::optional<MonoObject*> CallMethod(const Method& method, Args... args) const
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

				return CallMethodInternal(method, params);
			}
			std::optional<MonoObject*> CallMethod(const Method& method) const
			{
				return CallMethodInternal(method.GetPtr(), nullptr);
			}

			auto GetPtr() const { return m_object; }

			bool operator==(const Object& other) const { return m_object == other.m_object; }

		private:
			void GetValueInternal(const Field& field, void* value) const;
			void SetValueInternal(const Field& field, void* value) const;

			std::optional<MonoObject*> CallMethodInternal(MonoMethod* method, void* params[]) const;

		protected:
			MonoObject* m_object;
			std::optional<Class> m_class;
		};

	public:

		static void ReloadAssemblies(bool restoreData);

		// Name must be the full path, like : RexEngine.SomeClass::SomeMethod
		static void RegisterCall(const std::string& name, const void* function);

		static std::string GetString(MonoString* string);

		template<typename T>
		static T Unbox(MonoObject* obj)
		{
			return *(T*)UnboxInternal(obj);
		}
				
	private:
		static bool CheckMonoError(MonoError& error);
		static void* UnboxInternal(MonoObject* obj);

		static void Init();
		static void Stop();

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStart().Register<&Mono::Init>();
			EngineEvents::OnEngineStop().Register<&Mono::Stop>();
		})

	private:
		inline static MonoDomain* s_appDomain;
		inline static MonoDomain* s_rootDomain;

		RE_DECL_EVENT(OnReloadStart);
	};
}