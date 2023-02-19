#include "REPch.h"
#include "LuaApi.h"

#include "scene/Components.h"

namespace RexEngine
{
	void LuaApi::CreateScript(const std::string& type, Entity parent)
	{
		lua_getglobal(LuaEngine::GetCurrentState(), "Internal_AddComponent");
		if (LuaEngine::CallLuaMember<LuaNil>(type, "new").first) // LuaNil will leave the return on the stack
		{
			// The value is on the stack
			// TODO : Bad, should not be strings (userdata ?)
			LuaEngine::PushStack<LuaString>(LuaEngine::GetCurrentState(), parent.GetGuid().ToString()); // Push the key on the stack
			LuaEngine::PushStack<LuaString>(LuaEngine::GetCurrentState(), type);

			if (LuaEngine::CheckLua(LuaEngine::GetCurrentState(), lua_pcall(LuaEngine::GetCurrentState(), 3, 0, 0)))
			{
				if (!parent.HasComponent<ScriptComponent>())
					parent.AddComponent<ScriptComponent>();

				parent.GetComponent<ScriptComponent>().AddScript(type);
				return;
			}
		}

		lua_pop(LuaEngine::GetCurrentState(), 1);
		RE_LOG_ERROR("Error creating a component of type {}", type);
	}

	void LuaApi::ForEachScript(Guid parentGuid, std::function<void()>)
	{
	}


	void LuaApi::OnLuaInit()
	{
		LuaEngine::RegisterFunction<&LuaApi::LRegisterComponent, LuaString>("RegisterComponent");
		LuaEngine::RegisterFunction<&LuaApi::LLogInfo, LuaString>("LogInfo");
		LuaEngine::RegisterFunction<&LuaApi::LLogWarn, LuaString>("LogWarn");
		LuaEngine::RegisterFunction<&LuaApi::LLogError, LuaString>("LogError");
	}

	void LuaApi::OnLuaStart()
	{
		// TODO : should be in a file... std::embed :(
		static constexpr const char* LuaApiCode = R"(
-- Components
Component = { }

function Component:new(o)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	return o
end

function Component:Update()
end

Components = { }

function Internal_AddComponent(component, parentGuid, componentGuid)
	Components[parentGuid] = Components[parentGuid] or { }
	
	local cTable = Components[parentGuid]
	cTable[componentGuid] = component
end

function Internal_DoUpdates()
	for k1,obj in pairs(Components) do
		for k2,c in pairs(obj) do
			c:Update()
		end
	end
end

-- Test
TestComponent = Component:new();
RegisterComponent("TestComponent");

t = TestComponent:new();
function TestComponent:Update()
	LogInfo("TestComponent Update")
end
		)";

		LuaEngine::RunString(LuaApiCode);
		RE_LOG_WARN("registered : {}", s_registeredScripts.size());
	}

	void LuaApi::OnUpdate()
	{
		LuaEngine::CallLuaFunction<LuaNil>("Internal_DoUpdates");
	}

	// Called by lua
	void LuaApi::LRegisterComponent(LuaString typeName)
	{
		const auto str = std::string(typeName);
		if (!s_registeredScripts.contains(str))
		{
			s_registeredScripts.insert(str);
		}
		else
		{
			RE_LOG_WARN("Component {} already registered !", str);
		}
	}

	void LuaApi::LLogInfo(LuaString logStr)
	{
		auto[line, func, file] = LuaEngine::GetCurrentLocation();
		Log::DispatchLog(Log::LogType::Info, line, func, file, std::string(logStr));
	}
	void LuaApi::LLogWarn(LuaString logStr)
	{
		auto [line, func, file] = LuaEngine::GetCurrentLocation();
		Log::DispatchLog(Log::LogType::Warning, line, func, file, std::string(logStr));
	}
	void LuaApi::LLogError(LuaString logStr)
	{
		auto [line, func, file] = LuaEngine::GetCurrentLocation();
		Log::DispatchLog(Log::LogType::Error, line, func, file, std::string(logStr));
	}
}