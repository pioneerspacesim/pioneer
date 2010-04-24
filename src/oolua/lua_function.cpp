
#	include "lua_function.h"
#   include "oolua_check_result.h"
#   include <cassert>
namespace OOLUA
{
	void Lua_function::bind_script(lua_State* const lua)
	{
		assert(lua);
		m_lua = lua;
	}
	bool Lua_function::call(int const& count)
	{
		int result = lua_pcall(m_lua,count,LUA_MULTRET,0);
		return INTERNAL::protected_call_check_result(m_lua,result);
	}

	void Lua_function::set_function(std::string const& func)
	{
		lua_getfield(m_lua, LUA_GLOBALSINDEX, func.c_str());
	}
	void Lua_function::set_function(Lua_func_ref const& func)
	{
		lua_rawgeti(m_lua, LUA_REGISTRYINDEX, func.ref() );
	}
}

