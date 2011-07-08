#include "oolua_error.h"
#include "oolua_config.h"

#include <string>

#if OOLUA_STORE_LAST_ERROR == 1

#   include "lua_includes.h"

namespace OOLUA
{
    char const last_error_string[] = {"oolua_last_error"};
	void push_error_id_str(lua_State* l)
	{
		lua_pushlstring(l, last_error_string, (sizeof(last_error_string)/sizeof(char))-1);
	}
    void reset_error_value(lua_State*l)
    {
        push_error_id_str(l);
        lua_pushnil(l);
        lua_settable(l, LUA_REGISTRYINDEX);
    }
    std::string get_last_error(lua_State*l)
    {
        push_error_id_str(l);
        lua_gettable(l, LUA_REGISTRYINDEX);
        std::string error;
        if( (! lua_isnil(l,-1 ) ) && (lua_type(l, -1) == LUA_TSTRING ))
            error = lua_tolstring(l,-1,0);
        lua_pop( l, 1);
        return error;
    }
    namespace INTERNAL
    {
        void set_error_from_top_of_stack(lua_State*l)
        {
            int error_index = lua_gettop(l);
            push_error_id_str(l);
            lua_pushvalue(l,error_index);
            lua_settable(l, LUA_REGISTRYINDEX);
        }
		void set_error_from_top_of_stack_and_pop_the_error(lua_State*l)
		{
			int error_index = lua_gettop(l);
            push_error_id_str(l);
            lua_pushvalue(l,error_index);
            lua_settable(l, LUA_REGISTRYINDEX);
			lua_pop(l,1);
		}
    }
}
#else
//default implementations 
struct lua_State;

namespace OOLUA
{
    void reset_error_value(lua_State* /*l*/)
    {
    }
    std::string get_last_error(lua_State* /*l*/)
    {
		return std::string();
    }
	namespace INTERNAL
    {
        void set_error_from_top_of_stack(lua_State*	/*l*/)
        {
        }
    }
}
#endif
