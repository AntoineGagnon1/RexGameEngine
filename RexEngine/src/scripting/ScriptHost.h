#pragma once

#include <string>
#include <functional>
#include <filesystem>

#include <dotnet/hostfxr.h>
#include <dotnet/coreclr_delegates.h>

namespace RexEngine
{
	template<typename R, typename...Args>
	using ScriptFunc = std::function<R __stdcall(Args...)>;

	class ScriptHost
	{
	public:

		// location : path to the folder of the runtimeconfig.json and .dll files
		ScriptHost(const std::filesystem::path& location, const std::string& assemblyName);
		~ScriptHost();

		// Returns a std::function to the managed function className.funcName
		// Will return std::function(nullptr) in case of an error while getting the function
		// This will add __stdcall to the function type if needed
		template<typename R, typename...Args>
		ScriptFunc<R, Args...> GetFunction(const std::string& assemblyName, const std::string& className, const std::string& funcName)
		{
			typedef R(CORECLR_DELEGATE_CALLTYPE* fnType)(Args...);

			fnType fn = (fnType)GetFunction(assemblyName, className, funcName);
			return ScriptFunc<R, Args...>(fn);
		}

	private:
		// className : namespace qualified name
		void* GetFunction(const std::string& assemblyName, const std::string& className , const std::string& funcName);

	private:
		const std::filesystem::path m_dllPath;
		const std::string m_assemblyName;

		hostfxr_handle m_handle;
		load_assembly_and_get_function_pointer_fn m_getDelegate;
	};
}