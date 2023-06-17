#include "REPch.h"
#include "MonoEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace RexEngine
{
    constexpr mono_bool mono_true = 1;
    constexpr mono_bool mono_false = 0;

    MonoAssembly* MonoEngine::LoadAssembly(const std::filesystem::path& path)
    {
        auto fileData = FileHelper::ReadAllBytes<char>(path);

        // Copy the data, we don't have to keep fileData alive after the call
        // this image is ONLY valid to load the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData.data(), (uint32_t)fileData.size(), mono_true, &status, mono_false);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            RE_LOG_ERROR("Error while loading a c# image : {}", errorMessage);
            return nullptr;
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, path.string().c_str(), &status, mono_false);
        mono_image_close(image);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            RE_LOG_ERROR("Error while loading a c# assembly : {}", errorMessage);
            return nullptr;
        }

        return assembly;
    }

    void PrintAssemblyTypes(MonoAssembly* assembly)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
            RE_LOG_INFO("{}.{}", nameSpace, name);
        }
    }
    void MonoEngine::Init()
	{
		mono_set_assemblies_path("mono/lib");

        s_rootDomain = mono_jit_init("RexRuntime");
        if (s_rootDomain == nullptr)
        {
            RE_LOG_ERROR("Could not start mono !");
            return;
        }

        std::string appDomainName = "RexEngine"; // char* ...
        s_appDomain = mono_domain_create_appdomain(appDomainName.data(), nullptr);
        if (s_appDomain == nullptr)
        {
            RE_LOG_ERROR("Could not create the appdomain !");
            return;
        }
        mono_domain_set(s_appDomain, true);

        auto test = LoadAssembly("mono/DotNetApi.dll");
        PrintAssemblyTypes(test);
	}

}