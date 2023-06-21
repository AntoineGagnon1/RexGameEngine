#include "REPch.h"
#include "MonoEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/tokentype.h>

namespace RexEngine
{
    constexpr mono_bool mono_true = 1;
    constexpr mono_bool mono_false = 0;

    MonoAssembly* MonoEngine::LoadAssembly(const std::filesystem::path& path, const std::string& name)
    {
        auto fileData = FileHelper::ReadAllBytes<char>(path);

        // Copy the data, we don't have to keep fileData alive after the call
        // this image is ONLY valid to load the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData.data(), (uint32_t)fileData.size(), mono_true, &status, mono_false);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            RE_LOG_ERROR("Mono - Error while loading a c# image : {}", errorMessage);
            return nullptr;
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, path.string().c_str(), &status, mono_false);
        mono_image_close(image);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            RE_LOG_ERROR("Mono - Error while loading a c# assembly : {}", errorMessage);
            return nullptr;
        }

        if (s_loadedAssemblies.contains(name))
        {
            RE_LOG_WARN("Mono - An assembly is already loaded with the name {}", name);
            return nullptr;
        }
        
        s_loadedAssemblies.insert({ name, assembly });
        return assembly;
    }

    MonoClass* MonoEngine::GetClass(MonoAssembly* assembly, const std::string& namespaceName, const std::string& className)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        MonoClass* class_ = mono_class_from_name(image, namespaceName.c_str(), className.c_str());

        if (class_ == nullptr)
        {
            RE_LOG_ERROR("Mono - Class {} not found in {}.{}", className, GetAssemblyName(assembly), namespaceName);
            return nullptr;
        }

        return class_;
    }

    MonoObject* MonoEngine::CreateObject(MonoClass* class_)
    {
        MonoObject* classInstance = mono_object_new(s_appDomain, class_); // Allocate the space

        if (classInstance == nullptr)
        {
            RE_LOG_ERROR("Mono - Failed to create an instance of {} !", GetClassName(class_));
            return nullptr;
        }

        mono_runtime_object_init(classInstance); // Call the constructor
        return classInstance;
    }

    MonoMethod* MonoEngine::GetMethod(MonoClass* class_, const std::string& methodName, int numArgs)
    {
        MonoMethod* method = mono_class_get_method_from_name(class_, methodName.c_str(), numArgs);

        if (method == nullptr)
        {
            RE_LOG_ERROR("Mono - No method {} with {} arguments found in {}", methodName, numArgs, GetClassName(class_));
            return nullptr;
        }

        return method;
    }

    MonoMethod* MonoEngine::TryGetMethod(MonoClass* class_, const std::string& methodName, int numArgs)
    {
        return mono_class_get_method_from_name(class_, methodName.c_str(), numArgs);
    }

    std::string MonoEngine::GetClassName(MonoClass* class_)
    {
        return mono_class_get_name(class_);
    }

    std::string MonoEngine::GetClassNamespace(MonoClass* class_)
    {
        return mono_class_get_namespace(class_);
    }

    MonoClass* MonoEngine::GetParent(MonoClass* class_)
    {
        return mono_class_get_parent(class_);
    }

    std::vector<MonoClassField*> MonoEngine::GetFields(MonoClass* class_)
    {
        std::vector<MonoClassField*> fields;
        void* iter = nullptr;
        MonoClassField* field;

        while (field = mono_class_get_fields(class_, &iter), field != nullptr)
        {
            fields.push_back(field);
        }
        return fields;
    }

    std::string MonoEngine::GetFieldName(MonoClassField* field)
    {
        return mono_field_get_name(field);
    }

    std::type_index MonoEngine::GetFieldType(MonoClassField* field)
    {
        if (field == nullptr)
            return typeid(void);

        // TODO : Add support for more types, figure-out how to do string and char
        int typeID = mono_type_get_type(mono_field_get_type(field));
        switch (typeID)
        {
        case MONO_TYPE_VOID: return typeid(void);
        case MONO_TYPE_BOOLEAN: return typeid(bool);

        case MONO_TYPE_I1: return typeid(int8_t);
        case MONO_TYPE_I2: return typeid(int16_t);
        case MONO_TYPE_I4: return typeid(int32_t);
        case MONO_TYPE_I8: return typeid(int64_t);
        case MONO_TYPE_U1: return typeid(uint8_t);
        case MONO_TYPE_U2: return typeid(uint16_t);
        case MONO_TYPE_U4: return typeid(uint32_t);
        case MONO_TYPE_U8: return typeid(uint64_t);

        case MONO_TYPE_R4: return typeid(float);
        case MONO_TYPE_R8: return typeid(double);
        }
        return typeid(void);
    }

    MonoClass* MonoEngine::GetClass(MonoObject* obj)
    {
        return mono_object_get_class(obj);
    }

    MonoAssembly* MonoEngine::GetAssembly(const std::string& name)
    {
        if (!s_loadedAssemblies.contains(name))
        {
            RE_LOG_WARN("Mono - No assembly named {} is loaded", name);
            return nullptr;
        }

        return s_loadedAssemblies[name];
    }

    std::string_view MonoEngine::GetAssemblyName(MonoAssembly* assembly)
    {
        for (auto&& pair : s_loadedAssemblies)
        {
            if (pair.second == assembly)
                return pair.first;
        }

        return "AssemblyNameNotFound";
    }

    std::vector<MonoAssembly*> MonoEngine::GetAssemblies()
    {
        std::vector<MonoAssembly*> assemblies;
        assemblies.reserve(s_loadedAssemblies.size());
        for (auto&& pair : s_loadedAssemblies)
            assemblies.push_back(pair.second);
        return assemblies;
    }

    std::vector<MonoClass*> MonoEngine::GetTypes(MonoAssembly* assembly)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        std::vector<MonoClass*> types;
        types.reserve(numTypes);
    
        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
            auto class_ = mono_class_from_name(image, nameSpace, name);
            
            if (class_ != nullptr)
            {
                types.push_back(class_);
            }
        }
        return types;
    }

    MonoCustomAttrInfo* MonoEngine::GetAttributes(MonoClass* class_, MonoClassField* field)
    {
        return mono_custom_attrs_from_field(class_, field);
    }

    bool MonoEngine::ContainsAttribute(MonoCustomAttrInfo* attributes, MonoClass* attribute)
    {
        if (attributes == nullptr || attribute == nullptr)
            return false;

        for (int i = 0; i < attributes->num_attrs; i++)
        {
            if (mono_method_get_class(attributes->attrs[i].ctor) == attribute)
                return true;
        }
        return false;
    }

    void MonoEngine::RegisterCall(const std::string& name, const void* function)
    {
        mono_add_internal_call(name.c_str(), function);
    }

    std::string MonoEngine::GetString(MonoString* string)
    {
        if (string == nullptr || mono_string_length(string) == 0)
            return "";

        MonoError error;
        char* utf8 = mono_string_to_utf8_checked(string, &error);
        if (CheckMonoError(error))
            return "";
        std::string result(utf8);
        mono_free(utf8);
        return result;
    }

    std::optional<MonoObject*> MonoEngine::CallMethodInternal(MonoMethod* method, void* thisPtr, void* params[])
    {
        MonoObject* exception = nullptr;
        MonoObject* val = mono_runtime_invoke(method, thisPtr, params, &exception);
        if (exception != nullptr)
        {
            RE_LOG_ERROR("Mono - Could not call the method {} : {}", mono_method_get_name(method), GetExceptionMessage(exception));
            return {};
        }

        return val;
    }

    std::string MonoEngine::GetExceptionMessage(MonoObject* exception)
    {
        auto method = GetMethod(mono_get_exception_class(), "ToString", 0);
        return GetString((MonoString*)CallMethod(method, exception).value());
    }

    bool MonoEngine::CheckMonoError(MonoError& error)
    {
        bool hasError = !mono_error_ok(&error);
        if (hasError)
        {
            unsigned short errorCode = mono_error_get_error_code(&error);
            const char* errorMessage = mono_error_get_message(&error);
            RE_LOG_ERROR("Mono - Error #{}:{}", errorCode, errorMessage);
            mono_error_cleanup(&error);
        }
        return hasError;
    }

    void* MonoEngine::UnboxInternal(MonoObject* obj)
    {
        return mono_object_unbox(obj);
    }

    void MonoEngine::GetFieldValueInternal(MonoObject* instance, MonoClassField* field, void* value)
    {
        mono_field_get_value(instance, field, value);
    }

    void MonoEngine::SetFieldValueInternal(MonoObject* instance, MonoClassField* field, void* value)
    {
        mono_field_set_value(instance, field, value);
    }

    void* MonoEngine::GetMethodThunkInternal(MonoMethod* method)
    {
        return mono_method_get_unmanaged_thunk(method);
    }

    void MonoEngine::Init()
	{
		mono_set_assemblies_path("mono/lib");

        s_rootDomain = mono_jit_init("RexRuntime");
        if (s_rootDomain == nullptr)
        {
            RE_LOG_ERROR("Mono - Could not start mono !");
            return;
        }

        std::string appDomainName = "RexEngine"; // char* ...
        s_appDomain = mono_domain_create_appdomain(appDomainName.data(), nullptr);
        if (s_appDomain == nullptr)
        {
            RE_LOG_ERROR("Mono - Could not create the appdomain !");
            return;
        }
        mono_domain_set(s_appDomain, true);

        mono_assembly_foreach([](auto assembly, [[maybe_unused]]auto user_data) {
            s_mainAssembly = (MonoAssembly*)assembly;
            }, nullptr);

        OnMonoStart().Dispatch();
	}

    void MonoEngine::Stop()
    {
        mono_jit_cleanup(s_rootDomain);
    }

}