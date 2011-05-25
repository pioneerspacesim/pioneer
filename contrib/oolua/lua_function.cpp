
#	include "lua_function.h"
#   include "oolua_check_result.h"
#	include "oolua_config.h"

#if OOLUA_DEBUG_CHECKS == 1
#   include <cassert>
#endif

//#if OOLUA_DEBUG_CHECKS == 1
namespace 
{
	const int LEVELS1 = 10;
	const int LEVELS2 = 20;
	
	lua_State *getthread (lua_State *L, int *arg) {
		if (lua_isthread(L, 1)) {
			*arg = 1;
			return lua_tothread(L, 1);
		}
		else {
			*arg = 0;
			return L;
		}
	}
	
	int stack_trace (lua_State *L) {
		int level;
		int firstpart = 1;  /* still before eventual `...' */
		int arg;
		lua_State *L1 = getthread(L, &arg);
		lua_Debug ar;
		if (lua_isnumber(L, arg+2)) {
			level = (int)lua_tointeger(L, arg+2);
			lua_pop(L, 1);
		}
		else
			level = (L == L1) ? 1 : 0;  /* level 0 may be this own function */
		if (lua_gettop(L) == arg)
			lua_pushliteral(L, "");
		else if (!lua_isstring(L, arg+1)) return 1;  /* message is not a string */
		else lua_pushliteral(L, "\n");
		lua_pushliteral(L, "stack traceback:");
		while (lua_getstack(L1, level++, &ar)) {
			if (level > LEVELS1 && firstpart) {
				/* no more than `LEVELS2' more levels? */
				if (!lua_getstack(L1, level+LEVELS2, &ar))
					level--;  /* keep going */
				else {
					lua_pushliteral(L, "\n\t...");  /* too many levels */
					while (lua_getstack(L1, level+LEVELS2, &ar))  /* find last levels */
						level++;
				}
				firstpart = 0;
				continue;
			}
			lua_pushliteral(L, "\n\t");
			lua_getinfo(L1, "Snl", &ar);
			lua_pushfstring(L, "%s:", ar.short_src);
			if (ar.currentline > 0)
				lua_pushfstring(L, "%d:", ar.currentline);
			if (*ar.namewhat != '\0')  /* is there a name? */
				lua_pushfstring(L, " in function " LUA_QS, ar.name);
			else {
				if (*ar.what == 'm')  /* main? */
					lua_pushfstring(L, " in main chunk");
				else if (*ar.what == 'C' || *ar.what == 't')
					lua_pushliteral(L, " ?");  /* C function or tail call */
				else
					lua_pushfstring(L, " in function <%s:%d>",
									ar.short_src, ar.linedefined);
			}
			lua_concat(L, lua_gettop(L) - arg);
		}
		lua_concat(L, lua_gettop(L) - arg);
		return 1;
	}

}
//#endif

namespace OOLUA
{
	int set_error_callback(lua_State* l, lua_CFunction func)
	{
#if OOLUA_DEBUG_CHECKS == 1
		lua_pushcfunction(l, func);
		return lua_gettop(l);
#else
		(void)l;
		(void)func;
		return 0;
#endif
	}
	void remove_callback(lua_State* l, int index)
	{
		if (index != 0)lua_remove(l, index);
	}
	void Lua_function::bind_script(lua_State* const lua)
	{
#if OOLUA_DEBUG_CHECKS == 1
		assert(lua);
#endif
		m_lua = lua;
	}
	bool Lua_function::call(int const& count)
	{
		int result = lua_pcall(m_lua,count,LUA_MULTRET,m_error_func_index);
		remove_callback(m_lua,m_error_func_index);
		return INTERNAL::protected_call_check_result(m_lua,result);
	}

	void Lua_function::set_function(std::string const& func)
	{
		m_error_func_index = set_error_callback(m_lua,stack_trace);
		//REMOVE
		//lua_getfield(m_lua, LUA_GLOBALSINDEX, func.c_str());
		lua_getglobal(m_lua,func.c_str() );
	}
	void Lua_function::set_function(Lua_func_ref const& func)
	{
		m_error_func_index = set_error_callback(m_lua,stack_trace);
		lua_rawgeti(m_lua, LUA_REGISTRYINDEX, func.ref() );
	}
}

