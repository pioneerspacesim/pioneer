#include "push_pointer_internal.h"
#include "oolua_config.h"

namespace OOLUA
{
	namespace INTERNAL
	{
#if OOLUA_DEBUG_CHECKS == 1
		void if_check_enabled_check_type_is_registered(lua_State* l, char const* name)
		{
			lua_getfield(l, LUA_REGISTRYINDEX, name);
			if( lua_isnil(l,-1) )
			{
				lua_pop(l,1);
				luaL_error(l,"%s %s %s","the type",name,"is not registered with this Lua State");
			}
			else lua_pop(l,1);
		}
#else
		void if_check_enabled_check_type_is_registered(lua_State* /*l*/, char const* /*name*/){}
#endif
		
		void set_owner_if_change(Owner owner, Lua_ud* ud)
		{
			if(owner != No_change)
			{
				ud->gc =( owner == Lua ? true : false);
			}
		}
		
	}
}