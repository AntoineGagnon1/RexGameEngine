#include <REPch.h>
#include "Mono.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/tokentype.h>

namespace RexEngine
{
    std::optional<Mono::Class> Mono::Assembly::GetClass(const std::string& namespaceName, const std::string& className) const
    {
        MonoImage* image = mono_assembly_get_image(m_assembly);
        MonoClass* class_ = mono_class_from_name(image, namespaceName.c_str(), className.c_str());

        if (class_ == nullptr)
        {
            RE_LOG_ERROR("Mono - Class {} not found in {}.{}", className, GetName(), namespaceName);
            return {};
        }

        return Class(class_);
    }

    std::string Mono::Assembly::GetName() const
    {
        return mono_assembly_name_get_name(mono_assembly_get_name(m_assembly));
    }

    std::vector<Mono::Class> Mono::Assembly::GetTypes() const
    {
        MonoImage* image = mono_assembly_get_image(m_assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        std::vector<Class> types;
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
                types.push_back(Class(class_));
            }
        }
        return types;
    }

    std::optional<Mono::Method> Mono::Class::TryGetMethod(const std::string& methodName, int numArgs) const
    {
        MonoMethod* method = mono_class_get_method_from_name(m_class, methodName.c_str(), numArgs);
        if (method == nullptr)
            return {};
        else 
            return Method(method);
    }

    std::string Mono::Class::Name() const
    {
        return mono_class_get_name(m_class);
    }

    std::string Mono::Class::Namespace() const
    {
        return mono_class_get_namespace(m_class);
    }

    std::optional<Mono::Class> Mono::Class::Parent() const
    {
        auto parent = mono_class_get_parent(m_class);
        if (parent == nullptr)
            return {};
        else
            return Class(parent);
    }

    std::vector<Mono::Field> Mono::Class::Fields() const
    {
        std::vector<Field> fields;
        void* iter = nullptr;
        MonoClassField* field;

        while (field = mono_class_get_fields(m_class, &iter), field != nullptr)
        {
            fields.push_back(Field(field));
        }
        return fields;
    }

    void* Mono::Method::GetThunkInternal() const
    {
        return mono_method_get_unmanaged_thunk(m_method);
    }

    std::string Mono::Field::Name() const
    {
        return mono_field_get_name(m_field);
    }

    std::type_index Mono::Field::Type() const
    {
        // TODO : Add support for more types, figure-out how to do string and char
        int typeID = mono_type_get_type(mono_field_get_type(m_field));
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

    bool Mono::Field::HasAttribute(const Class& attributeType) const
    {
        auto attributes = mono_custom_attrs_from_field(mono_field_get_parent(m_field), m_field);

        if (attributes == nullptr)
            return false;

        for (int i = 0; i < attributes->num_attrs; i++)
        {
            if (mono_method_get_class(attributes->attrs[i].ctor) == attributeType.GetPtr())
                return true;
        }
        return false;
    }


    std::optional<Mono::Object> Mono::Object::Create(const Class& class_)
    {
        MonoObject* classInstance = mono_object_new(s_appDomain, class_.GetPtr()); // Allocate the space

        if (classInstance == nullptr)
        {
            RE_LOG_ERROR("Mono - Failed to create an instance of {} !", class_.Name());
            return {};
        }

        mono_runtime_object_init(classInstance); // Call the constructor
        return Object(classInstance, class_);
    }

    void Mono::Object::GetValueInternal(const Field& field, void* value) const
    {
        mono_field_get_value(m_object, field.GetPtr(), value);
    }

    void Mono::Object::SetValueInternal(const Field& field, void* value) const
    {
        mono_field_set_value(m_object, field.GetPtr(), value);
    }

    std::optional<MonoObject*> Mono::Object::CallMethodInternal(MonoMethod* method, void* params[]) const
    {
        MonoObject* exception = nullptr;
        MonoObject* val = mono_runtime_invoke(method, m_object, params, &exception);
        if (exception != nullptr)
        {
            // TODO : find a better way to get the message
            auto method2 = mono_class_get_method_from_name(mono_get_exception_class(), "ToString", 0);
            MonoObject* exception2 = nullptr;
            auto msg = mono_runtime_invoke(method2, exception, nullptr, &exception2);
            RE_LOG_ERROR("Mono - Could not call the method {} : {}", mono_method_get_name(method), Mono::GetString((MonoString*)msg));
            return {};
        }

        return val;
    }

    std::optional<Mono::Assembly> Mono::LoadAssembly(const std::filesystem::path& dllPath)
    {
        auto fileData = FileHelper::ReadAllBytes<char>(dllPath);

        // Copy the data, we don't have to keep fileData alive after the call
        // this image is ONLY valid to load the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData.data(), (uint32_t)fileData.size(), 1, &status, 0);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            RE_LOG_ERROR("Mono - Error while loading a c# image : {}", errorMessage);
            return {};
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, dllPath.string().c_str(), &status, 0);
        mono_image_close(image);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            RE_LOG_ERROR("Mono - Error while loading a c# assembly : {}", errorMessage);
            return {};
        }

        return Assembly(assembly);
    }

    std::string Mono::GetString(MonoString* string)
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

    void Mono::RegisterCall(const std::string& name, const void* function)
    {
        mono_add_internal_call(name.c_str(), function);
    }

    bool Mono::CheckMonoError(MonoError& error)
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

    void* Mono::UnboxInternal(MonoObject* obj)
    {
        return mono_object_unbox(obj);
    }

    void Mono::Init()
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

    void Mono::Stop()
    {
        mono_jit_cleanup(s_rootDomain);
    }

}