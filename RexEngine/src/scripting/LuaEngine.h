#pragma once
#include "../core/EngineEvents.h"

#include <type_traits>
#include <variant>

extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

// Interface : 
// General lua interface :
// - Call global function / member function
// - register global function / object
// 
// 




namespace RexEngine
{
	// Types used by lua
	using LuaNil = std::monostate;
	using LuaBool = bool;
	using LuaNumber = lua_Number;
	using LuaInteger = lua_Integer;
	using LuaString = std::string_view;
	// TODO : function, userdata, thread, table
	using LuaTypes = std::variant<LuaNil, LuaBool, LuaNumber, LuaInteger, LuaString>;


	class LuaEngine
	{
	public:
		RE_DECL_EVENT(OnLuaInit) // Called after the first time lua is started (not after reloads)
		RE_DECL_EVENT(OnLuaStart) // Called when lua is started (reloads included)

		// Reload the lua environment
		static void Reload();

		// Make a wrapper around a c++ function
		template<auto Func, typename ...Args>
		static lua_CFunction MakeLuaFunction();

		// Register a lua library
		static void RegisterLib(const std::string& name, std::span<luaL_Reg> functions);

		// Register a function to use in lua, Args are the type of the arguments the function will receive 
		template<auto Func, typename ...Args>
		static void RegisterFunction(const std::string& name);

		static void RunString(const std::string& str);


		// Call a global function in lua
		template<typename R, typename ...Args>
		static R CallLuaFunction(const std::string& name, Args&& ...args);

		// Call a member function in lua, the return type R can be LuaNil if the functions returns nothing
		// Returns <success ?, return value>
		template<typename R, typename ...Args>
		static std::pair<bool, R> CallLuaMember(const std::string& objName, const std::string& methodName, Args&& ...args);

		// Returns an ordered (lua order using next()) vector of all the fields in the table
		// IMPORTANT : to be considered a field the key must be a string
		// If the table does not exist an empty vector is returned and a message is logged
		static std::vector<std::pair<LuaString, LuaTypes>> GetTableFields(const std::string& tableName);

		// Calls the function for each field in the table, the first argument is the key, the second is the value
		// if the function returns false the loop stops
		static void ForEachTableField(const std::string& tableName, std::function<bool(LuaTypes, LuaTypes)> func);

		// Set the field of a global table to the value
		template<typename KeyT, typename ValT>
		static void SetGlobalTableField(const std::string& tableName, KeyT&& key, ValT&& value);

		// Get the debug source location of the lua code currently running
		// Line, FuncName, FileName
		static std::tuple<int, std::string, std::string> GetCurrentLocation();

		static lua_State* GetCurrentState() { return s_luaState; }

		template<typename Arg>
		static Arg PopStack(lua_State* L);

		template<typename Arg>
		static bool PushStack(lua_State* L, const Arg& val);

		static LuaTypes PeekStack(int index);


		// Used with function calls that return an int error code
		static bool CheckLua(lua_State* state, int r);

		static std::string GetStack();
	private:
		inline static lua_State* s_luaState = nullptr;

		inline static std::vector<std::pair<std::string, lua_CFunction>> s_registeredFunctions;

		static void Init();
		static void Start();
		static void Stop();


		// Get the name of the type of the item at the index on the stack
		static std::string_view GetStackType(lua_State* L, int index);

		// Make a lua call (the function needs to already be on top of the stack)
		template<typename R, typename ...Args>
		static R DoLuaCall(Args&& ...args);

		RE_STATIC_CONSTRUCTOR({
			EngineEvents::OnEngineStart().Register<&LuaEngine::Init>();
			EngineEvents::OnEngineStop().Register<&LuaEngine::Stop>();
		})
	};






	template<auto Func, typename ...Args>
	inline void LuaEngine::RegisterFunction(const std::string& name)
	{
		s_registeredFunctions.push_back({ name, MakeLuaFunction<Func, Args...>() });
		lua_register(s_luaState, name.c_str(), s_registeredFunctions.back().second);
	}

	template<auto Func, typename ...Args>
	static lua_CFunction LuaEngine::MakeLuaFunction()
	{
		using result = std::invoke_result_t<decltype(Func), Args...>;
	
		return [](lua_State* L) -> int {

			if constexpr (std::is_void_v<result>)
			{
				Func(PopStack<Args>(L)...);
				return 0;
			}
			else
			{
				PushStack<result>(L, Func(PopStack<Args>(L)...));
				return 1;
			}
		};
	}


	template<typename R, typename ...Args>
	inline R LuaEngine::CallLuaFunction(const std::string& name, Args&& ...args)
	{
		// Get the function on the stack
		lua_getglobal(s_luaState, name.c_str());

		return DoLuaCall<R, Args...>(std::forward<Args>(args)...);
	}

	template<typename R, typename ...Args>
	inline std::pair<bool, R> LuaEngine::CallLuaMember(const std::string& objName, const std::string& methodName, Args&& ...args)
	{
		// Get the object on the stack
		lua_getglobal(s_luaState, objName.c_str());

		// Get the method on the stack
		lua_getfield(s_luaState, -1, methodName.c_str());


		// Was the method found ?
		if (lua_isnil(s_luaState, -1))
		{ // Not found
			RE_LOG_ERROR("No method called {} in lua object {}", methodName, objName);
			lua_pop(s_luaState, 1);

			return {false, R()};
		}

		lua_pushvalue(s_luaState, -2); // Get the obj as the first argument (self)
		if constexpr (!std::is_same_v<R, LuaNil>)
		{
			auto value = DoLuaCall<R, LuaNil, Args...>(std::monostate(), std::forward<Args>(args)...);
			lua_pop(s_luaState, 1); // Pop the object from the stack
			return {true, value };
		}
		else
		{ // No return
			DoLuaCall<R, LuaNil, Args...>(std::monostate(), std::forward<Args>(args)...);
			return {true, R()};
		}

	}

	template<typename KeyT, typename ValT>
	inline void LuaEngine::SetGlobalTableField(const std::string& tableName, KeyT&& key, ValT&& value)
	{
		// Put the table on the stack
		lua_getglobal(s_luaState, tableName.c_str());

		PushStack<ValT>(std::forward<ValT>(value)); // Push the value on the stack
		PushStack<KeyT>(std::forward<KeyT>(key)); // Push the key on the stack
		
		lua_settable(s_luaState, -3);

		lua_pop(s_luaState, 1); // Pop the table from the stack
	}


	template<typename R, typename ...Args>
	inline R LuaEngine::DoLuaCall(Args&& ...args)
	{
		// Push all the arguments
		(PushStack<Args>(s_luaState, std::forward<Args>(args)), ...);

		if (CheckLua(s_luaState, lua_pcall(s_luaState, sizeof...(Args), std::is_void_v<R> ? 0 : 1, 0)))
		{
			// No errors, get the return value
			if constexpr (!std::is_void_v<R>)
			{
				return PopStack<R>(s_luaState);
			}
		}

		if constexpr (!std::is_void_v<R>)
			return R();
		else
			return;
	}

	template<typename Arg>
	inline Arg LuaEngine::PopStack(lua_State* L)
	{
		Arg result;

		if constexpr (std::is_same_v<Arg, LuaBool>)
		{
			RE_ASSERT(lua_isboolean(L, -1), "Invalid lua type on the stack, expected {}, got {}", typeid(Arg).name(), GetStackType(L, -1));
			result = static_cast<bool>(lua_toboolean(L, -1));
		}
		else if constexpr (std::is_integral_v<Arg>)
		{
			RE_ASSERT(lua_isinteger(L, -1), "Invalid lua type on the stack, expected {}, got {}", typeid(Arg).name(), GetStackType(L, -1));
			result = static_cast<Arg>(lua_tointeger(L, -1));
		}
		else if constexpr (std::is_floating_point_v<Arg>)
		{
			RE_ASSERT(lua_isnumber(L, -1), "Invalid lua type on the stack, expected {}, got {}", typeid(Arg).name(), GetStackType(L, -1));
			result = static_cast<Arg>(lua_tonumber(L, -1));
		}
		else if constexpr (std::is_same_v<Arg, LuaString>)
		{
			RE_ASSERT(lua_isstring(L, -1), "Invalid lua type on the stack, expected {}, got {}", typeid(Arg).name(), GetStackType(L, -1));
			size_t len = 0;
			auto str = lua_tolstring(L, -1, &len);
			result = LuaString(str, len);
		}
		else if constexpr (!std::is_same_v<Arg, LuaNil>) // Pop<Nil>() does not print an error
		{
			// TODO : fonction, userdata, thread, table
			RE_LOG_ERROR(false, "Type {} is not defined for lua", typeid(Arg).name());
			result = Arg();
		}

		lua_pop(L, 1);
		return result;
	}

	template<typename Arg>
	inline bool LuaEngine::PushStack(lua_State* L, const Arg& val)
	{
		if (!lua_checkstack(L, 1))
		{
			RE_LOG_ERROR("The lua stack is full !");
			return false;
		}

		if constexpr (std::is_same_v<Arg, LuaBool>)
		{
			lua_pushboolean(L, static_cast<int>(val));
		}
		else if constexpr (std::is_integral_v<Arg>)
		{
			lua_pushinteger(L, static_cast<lua_Integer>(val));
		}
		else if constexpr (std::is_floating_point_v<Arg>)
		{
			lua_pushnumber(L, static_cast<lua_Number>(val));
		}
		else if constexpr (std::is_same_v<Arg, LuaString>)
		{
			lua_pushlstring(L, val.data(), val.size());
		}
		else if constexpr (!std::is_same_v<Arg, LuaNil>) // Push<Nil>() does not print an error
		{
			// TODO : nil, fonction, userdata, thread, table
			RE_LOG_ERROR(false, "Type {} is not defined for lua", typeid(Arg).name());
			return false;
		}

		return true;
	}
}