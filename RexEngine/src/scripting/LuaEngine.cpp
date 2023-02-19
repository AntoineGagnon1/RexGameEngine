#include "REPch.h"
#include "LuaEngine.h"

#include <format>

namespace RexEngine
{
	void LuaEngine::Reload()
	{
		Stop();
		Start();
		OnLuaStart().Dispatch();
	}

	void LuaEngine::RegisterLib(const std::string& name, std::span<luaL_Reg> functions)
	{
		lua_createtable(s_luaState, 0, (int)functions.size());
		
		for (auto& p : functions)
		{
			lua_pushcfunction(s_luaState, p.func);
			lua_setfield(s_luaState, -2, p.name);
		}

		lua_setglobal(s_luaState, name.c_str());
	}

	void LuaEngine::RunString(const std::string& str)
	{
		CheckLua(s_luaState, luaL_dostring(s_luaState, str.c_str()));
	}

	std::vector<std::pair<LuaString, LuaTypes>> LuaEngine::GetTableFields(const std::string& tableName)
	{
		std::vector<std::pair<LuaString, LuaTypes>> result;

		// Get the table
		lua_getglobal(s_luaState, tableName.c_str());

		lua_pushnil(s_luaState); // the first key to start the iteration

		while (lua_next(s_luaState, -2) != 0) 
		{
			// The key is at index -2 and the value is at -1
			if (lua_type(s_luaState, -2) == LUA_TSTRING)
			{
				// Get the key as a LuaString
				size_t length = 0;
				auto ptr = lua_tolstring(s_luaState, -2, &length);
				auto key = LuaString(ptr, length);


				// Get the value type
				switch (lua_type(s_luaState, -1))
				{
				case LUA_TNIL:
					result.push_back({ key, LuaTypes(LuaNil()) });
					lua_pop(s_luaState, 1);
					break;
				case LUA_TBOOLEAN:
					result.push_back({ key, PopStack<LuaBool>(s_luaState) });
					break;
				case LUA_TNUMBER:
					result.push_back({ key, PopStack<LuaNumber>(s_luaState) });
					break;
				case LUA_TSTRING:
					result.push_back({ key, PopStack<LuaString>(s_luaState)});
					break;
					// TODO : more types
				default:
					break;
				}
			}
		}

		// Pop the table from the stack
		lua_pop(s_luaState, 1);

		return result;
	}

	void LuaEngine::ForEachTableField(const std::string& tableName, std::function<bool(LuaTypes, LuaTypes)> func)
	{
		// Get the table
		lua_getglobal(s_luaState, tableName.c_str());

		lua_pushnil(s_luaState); // the first key to start the iteration

		while (lua_next(s_luaState, -2) != 0)
		{
			auto key = PeekStack(-2); // Get the key, but keep it on the stack
			auto value = PeekStack(-1);
			lua_pop(s_luaState, 1); // Pop the value

			if (!func(key, value))
				break;
		}

		// Pop the table from the stack
		lua_pop(s_luaState, 1);
	}

	// Line, FuncName, FileName
	std::tuple<int, std::string, std::string> LuaEngine::GetCurrentLocation()
	{
		lua_Debug info;
		lua_getstack(s_luaState, 1, &info);
		lua_getinfo(s_luaState, "nSl", &info);
		
		std::string file = "";
		if (info.source[0] == '@' || info.source[0] == '=')
		{
			file = std::string(info.source).substr(1);
		}

		std::string func = "";
		if (info.namewhat[0] != '\0')
			func += std::string(info.namewhat) + " ";

		if (info.name != nullptr)
			func += info.name;

		return { info.currentline, func, file};
	}

	void LuaEngine::Init()
	{
		Start();
		OnLuaInit().Dispatch();
		OnLuaStart().Dispatch();
	}

	void LuaEngine::Start()
	{
		s_luaState = luaL_newstate();
		RE_ASSERT(s_luaState != nullptr, "Failed to start lua !");

		luaL_openlibs(s_luaState); // Add standard libraries

		// Register the functions already registered
		for (auto& pair : s_registeredFunctions)
		{
			lua_register(s_luaState, pair.first.c_str(), pair.second);
		}
	}

	LuaTypes LuaEngine::PeekStack(int index)
	{
		switch (lua_type(s_luaState, index))
		{
		case LUA_TBOOLEAN:
			return static_cast<LuaBool>(lua_toboolean(s_luaState, index));
		case LUA_TNUMBER:
			if(lua_isinteger(s_luaState, index))
				return static_cast<LuaInteger>(lua_tointeger(s_luaState, index));
			else
				return static_cast<LuaNumber>(lua_tonumber(s_luaState, index));
		case LUA_TSTRING:
			size_t len = 0;
			auto str = lua_tolstring(s_luaState, index, &len);
			return LuaString(str, len);
		}

		return LuaNil();
	}

	bool LuaEngine::CheckLua(lua_State* state, int r)
	{
		if (r != LUA_OK)
		{
			// TODO : add line and function info using debug hook
			Log::LogEvent().Dispatch(Log::LogType::Error, lua_tostring(state, -1), 0, "Lua", "Lua");
			return false;
		}
		return true;
	}

	std::string LuaEngine::GetStack()
	{
		std::string str = "Lua Stack\n";
		int top = lua_gettop(s_luaState);
		for (unsigned int index = top; index > 0; index--)
		{
			int type = lua_type(s_luaState, index);
			switch (type)
			{
			case LUA_TBOOLEAN:
				str += std::format("{:2}|{}\n", index, (bool)lua_toboolean(s_luaState, index));
				break;
			case LUA_TNUMBER:
				str += std::format("{:2}|{}\n", index, lua_tonumber(s_luaState, index));
				break;
			case LUA_TSTRING:
				str += std::format("{:2}|{}\n", index, lua_tostring(s_luaState, index));
				break;
			default:
				str += std::format("{:2}|{}\n", index, lua_typename(s_luaState, type));
				break;
			}
		}

		return str;
	}

	void LuaEngine::Stop()
	{
		if (s_luaState != nullptr)
			lua_close(s_luaState);


	}

	std::string_view LuaEngine::GetStackType(lua_State* L, int index)
	{
		return lua_typename(L, lua_type(L, index));
	}
}