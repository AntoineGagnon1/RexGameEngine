#include "REPch.h"
#include "MonoEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

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

    std::string_view MonoEngine::GetClassName(MonoClass* class_)
    {
        return mono_class_get_name(class_);
    }

    MonoClass* MonoEngine::GetParent(MonoClass* class_)
    {
        return mono_class_get_parent(class_);
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
            
            if(class_ != nullptr)
                types.push_back(class_);
        }
        return types;
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

        OnMonoStart().Dispatch();
	}

    void MonoEngine::Stop()
    {
        mono_jit_cleanup(s_rootDomain);
    }

}