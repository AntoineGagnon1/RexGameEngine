#pragma once

#include "LuaEngine.h"
#include "../scene/Entity.h"

namespace RexEngine
{
	// L prefix on functions is to indicate that the function is called by lua
	class LuaApi
	{
	public:
		
		// Create a lua script (the type must be registered)
		static void CreateScript(const std::string& type, Entity parent);

		static const auto& GetRegisteredScripts() { return s_registeredScripts; }

		// Every script will be put on the stack before the call to 
		// Return false from func to stop the iteration
		static void ForEachScript(Guid parentGuid, std::function<bool()> func);

	private:
		inline static std::unordered_set<std::string> s_registeredScripts;

	private:
		// Register all the c++ functions
		static void OnLuaInit();

		static void OnLuaStart();

		static void OnUpdate();

		// Called by lua to register a component type
		static void LRegisterComponent(LuaString typeName);

		// Logs from lua
		static void LLogInfo(LuaString logStr);
		static void LLogWarn(LuaString logStr);
		static void LLogError(LuaString logStr);

	
		RE_STATIC_CONSTRUCTOR({
			LuaEngine::OnLuaInit().Register<&LuaApi::OnLuaInit>();
			LuaEngine::OnLuaStart().Register<&LuaApi::OnLuaStart>();
			EngineEvents::OnUpdate().Register<&LuaApi::OnUpdate>();
		})
	};
}