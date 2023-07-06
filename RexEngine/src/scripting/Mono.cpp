#include <REPch.h>
#include "Mono.h"

#include "ScriptComponent.h"
#include <scene/Scene.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/tokentype.h>

namespace RexEngine
{
    Mono::Assembly::Assembly(const std::filesystem::path& dllPath)
        : m_dllPath(dllPath)
    {
        s_assemblies.push_back(this);
        Reload();
        // Assemblies have to reload the class list when a reload occurs
        Mono::OnReloadStart().Register<&Assembly::Reload>(this);
    }

    Mono::Assembly::~Assembly()
    {
        if(s_assemblies.size() > 0)
            s_assemblies.erase(std::remove(s_assemblies.begin(), s_assemblies.end(), this), s_assemblies.end());
        OnReloadStart().UnRegister<&Assembly::Reload>(this);
    }

    std::unique_ptr<Mono::Assembly> Mono::Assembly::Load(const std::filesystem::path& dllPath)
    {
        return std::unique_ptr<Assembly>(new Assembly(dllPath));
    }

    std::optional<Mono::Class> Mono::Assembly::GetClass(const std::string& namespaceName, const std::string& className) const
    {
        if (!m_classes.contains({ namespaceName, className }))
            return {};
        return Class(m_classes.at({namespaceName, className}).get());
    }

    std::string Mono::Assembly::GetName() const
    {
        return mono_assembly_name_get_name(mono_assembly_get_name(m_assembly));
    }

    std::vector<Mono::Class> Mono::Assembly::GetTypes() const
    {
        std::vector<Class> types;
        types.reserve(m_classes.size());

        for (auto&& [key, value] : m_classes)
        {
            types.emplace_back(value.get());
        }

        return types;
    }

    std::optional<Mono::Class> Mono::Assembly::FindClass(const std::string& namespaceName, const std::string& className)
    {
        for (Assembly* assembly : s_assemblies)
        {
            auto class_ = assembly->GetClass(namespaceName.c_str(), className.c_str());
            if (class_.has_value())
                return class_;
        }
        return {};
    }

    void Mono::Assembly::Reload()
    {
        m_assembly = LoadAssemblyInternal(m_dllPath);

        std::unordered_set<std::tuple<std::string, std::string>> removedClasses;
        for (auto&& [name, class_] : m_classes)
        {
            removedClasses.insert(name);
        }

        MonoImage* image = mono_assembly_get_image(m_assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        const int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        std::vector<ClassProxy_*> toAdd;
        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
            const auto class_ = mono_class_from_name(image, nameSpace, name);

            const auto key = std::tuple<std::string, std::string>(nameSpace, name);

            if (!m_classes.contains(key))
            {
                const auto data = m_classes.emplace(key, std::make_unique<ClassProxy_>(class_));
                toAdd.push_back(data.first->second.get());
            }
            else
            {
                removedClasses.erase(key);
                m_classes[key]->ptr = class_;
            }
        }

        for (auto& class_ : toAdd)
        {
            Mono::OnAddClass().Dispatch(*this, Class(class_));
        }

        for (auto& class_ : removedClasses)
        {
            Mono::OnRemoveClass().Dispatch(*this, Class(m_classes[class_].get()));
            m_classes.erase(class_);
        }
    }

    MonoAssembly* Mono::Assembly::LoadAssemblyInternal(const std::filesystem::path& dllPath)
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
            return nullptr;
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, dllPath.string().c_str(), &status, 0);
        mono_image_close(image);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            RE_LOG_ERROR("Mono - Error while loading a c# assembly : {}", errorMessage);
            return nullptr;
        }

        return assembly;
    }

    std::optional<Mono::Method> Mono::Class::TryGetMethod(const std::string& methodName, int numArgs) const
    {
        MonoClass* current = m_class->ptr;
        do
        {
            MonoMethod* method = mono_class_get_method_from_name(current, methodName.c_str(), numArgs);
            if (method != nullptr)
                return Method(method);

            current = mono_class_get_parent(current);
        } while (current != nullptr);

        return {};
    }

    std::string Mono::Class::Name() const
    {
        return mono_class_get_name(m_class->ptr);
    }

    std::string Mono::Class::Namespace() const
    {
        return mono_class_get_namespace(m_class->ptr);
    }

    bool Mono::Class::IsSubClassOf(const Class& parent)
    {
        MonoClass* current = m_class->ptr;
        do
        {
            current = mono_class_get_parent(current);
            if (current == parent.GetPtr())
                return true;

        } while (current != nullptr);

        return false;
    }

    std::vector<Mono::Field> Mono::Class::Fields() const
    {
        std::vector<Field> fields;
        void* iter = nullptr;
        MonoClassField* field;

        while (field = mono_class_get_fields(m_class->ptr, &iter), field != nullptr)
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

    void Mono::ReloadAssemblies(bool restoreData)
    {
        // Save all the c# fields
        std::vector<std::pair<ScriptComponent*, std::stringstream>> dataCache;
        if (restoreData)
        {
            auto scene = Scene::CurrentScene();
            if (scene)
            {
                for (auto&& [e, c] : scene->GetComponents<ScriptComponent>())
                {
                    std::stringstream stream;
                    {
                        JsonSerializer archive(stream);
                        archive(c);
                    }
                    dataCache.emplace_back(&c, std::move(stream));
                }
            }
        }

        mono_domain_set(s_rootDomain, false);
        mono_domain_unload(s_appDomain);

        std::string appDomainName = "RexEngine"; // char* ...
        s_appDomain = mono_domain_create_appdomain(appDomainName.data(), nullptr);
        mono_domain_set(s_appDomain, true);

        OnReloadStart().Dispatch();
        OnReload().Dispatch();

        // Load all the c# fields from the save
        for (auto& pair : dataCache)
        {
            JsonDeserializer archive(pair.second);
            archive(*pair.first);
        }
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

    MonoString* Mono::MakeString(const std::string& string)
    {
        return mono_string_new_len(s_appDomain, string.c_str(), (int)string.size());
    }

    void Mono::RegisterCallInternal(const std::string& name, const void* function)
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