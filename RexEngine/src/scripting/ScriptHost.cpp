#include "REPch.h"
#include "ScriptHost.h"

#include <dotnet/nethost.h>
#include <dotnet/coreclr_delegates.h>
#include <dotnet/hostfxr.h>

// Documentation for nethost at : https://github.com/dotnet/runtime/blob/main/docs/design/features/native-hosting.md
// Some code from : https://github.com/dotnet/samples/blob/main/core/hosting/src/NativeHost/nativehost.cpp

using string_t = std::basic_string<char_t>; // char_t is in nethost.h

#ifdef RE_WINDOWS
    #include <Windows.h>

    namespace RexEngine::Internal
    {
        string_t ToScriptString(const std::string& from)
        {
            // Convert from string to wstring
            string_t output(from.size(), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, from.c_str(), (int)from.size(), output.data(), (int)output.size());
            return output;
        }
    }

#else
    #include <dlfcn.h>
    #include <limits.h>

    namespace RexEngine::Internal
    {
        string_t ToScriptString(const std::string& from)
        {
            return from;
        }
    }

    #define MAX_PATH 260 // Already defined on windows

#endif

namespace RexEngine::Internal
{
#ifdef RE_WINDOWS
    void* Load_Library(const char_t* path) // LoadLibrary is a Windows macro
    {
        HMODULE h = ::LoadLibraryW(path);
        assert(h != nullptr);
        return (void*)h;
    }
    void* GetExport(void* h, const char* name)
    {
        void* f = ::GetProcAddress((HMODULE)h, name);
        assert(f != nullptr);
        return f;
    }
#else
    void* Load_Library(const char_t* path)
    {
        void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        assert(h != nullptr);
        return h;
    }
    void* GetExport(void* h, const char* name)
    {
        void* f = dlsym(h, name);
        assert(f != nullptr);
        return f;
    }
#endif

    hostfxr_initialize_for_runtime_config_fn initAssembly = nullptr;
    hostfxr_get_runtime_delegate_fn getDelegate = nullptr;
    hostfxr_close_fn closeAssembly = nullptr;

	bool LoadHostFxr()
	{
        static bool result = false;

        // Run once :
        if (static auto called = false; !std::exchange(called, true)) 
        {    
            // Pre-allocate a large buffer for the path to hostfxr
            char_t buffer[MAX_PATH];
            size_t buffer_size = sizeof(buffer) / sizeof(char_t);
            int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);

            RE_ASSERT(rc == 0, "Could not find hostfxr !");

            // Load hostfxr and get exports
            void* lib = Load_Library(buffer);
            initAssembly = (hostfxr_initialize_for_runtime_config_fn)GetExport(lib, "hostfxr_initialize_for_runtime_config");
            getDelegate = (hostfxr_get_runtime_delegate_fn)GetExport(lib, "hostfxr_get_runtime_delegate");
            closeAssembly = (hostfxr_close_fn)GetExport(lib, "hostfxr_close");

            RE_ASSERT(initAssembly, "Could not get hostfxr_initialize_for_runtime_config");
            RE_ASSERT(getDelegate, "Could not get hostfxr_get_runtime_delegate");
            RE_ASSERT(closeAssembly, "Could not get hostfxr_close");

            result = (initAssembly && getDelegate && closeAssembly);
        }

        return result;
	}

    // will return nullptr if the init did not work
    hostfxr_handle InitAssembly(const std::string& path)
    {
        if (!LoadHostFxr())
            return nullptr;

        hostfxr_handle handle = nullptr;

        int rc = initAssembly(ToScriptString(path).c_str(), nullptr, &handle);
        if (rc != 0)
            return  nullptr;

        return handle;
    }

    // Returns false if something went wrong
    bool CloseAssembly(hostfxr_handle handle)
    {
        return closeAssembly(handle) == 0;
    }

    load_assembly_and_get_function_pointer_fn GetDelegateFunction(hostfxr_handle handle)
    {
        void* result = nullptr;
        int rc = getDelegate( handle, hdt_load_assembly_and_get_function_pointer, &result);

        if (rc != 0)
            return (load_assembly_and_get_function_pointer_fn)nullptr;

        return (load_assembly_and_get_function_pointer_fn)result;
    }
}

namespace RexEngine
{
	ScriptHost::ScriptHost(const std::filesystem::path& location, const std::string& assemblyName)
        : m_getDelegate(nullptr), m_assemblyName(assemblyName), m_dllPath(location / std::filesystem::path(assemblyName + ".dll"))
	{
        m_handle = Internal::InitAssembly((location / std::filesystem::path(assemblyName + ".runtimeconfig.json")).string());
        if (!m_handle)
        { 
            RE_LOG_ERROR("Could not load the assembly [{}] at {}", assemblyName, location.string());
            return;
        }

        m_getDelegate = Internal::GetDelegateFunction(m_handle);
        if(!m_getDelegate)
            RE_LOG_ERROR("Could not get load_assembly_and_get_function_pointer_fn for [{}] at {}", assemblyName, location.string());
	}

    ScriptHost::~ScriptHost()
    {
        if (m_handle && !Internal::CloseAssembly(m_handle))
            RE_LOG_ERROR("Could not close the assembly [{}] at {}", m_assemblyName, m_dllPath.string());
    }


    void* ScriptHost::GetFunction(const std::string& assemblyName, const std::string& className, const std::string& funcName)
    {
        if (!m_getDelegate)
            return nullptr;

        typedef void (CORECLR_DELEGATE_CALLTYPE* func_t)(void);
        func_t functionPtr = nullptr;

        int rc = m_getDelegate(
            Internal::ToScriptString(m_dllPath.string()).c_str(), // Path to dll
            Internal::ToScriptString(std::format("{0}, {1}", className, assemblyName)).c_str(), // ex : "ClassLibrary1.Class1, ClassLibrary1"
            Internal::ToScriptString(funcName).c_str(), // method name
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            (void**)&functionPtr);

        if (rc != 0)
            return nullptr;

        return functionPtr;
    }
}