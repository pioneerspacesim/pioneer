#include "lua_includes.h"
#include "lua_stack_dump.h"
#include <iostream>

namespace OOLUA
{

void lua_stack_dump (lua_State * const L)
{
	int i;

	int top = lua_gettop(L);
	std::cout <<"Lua stack dump - number of nodes: " <<top <<std::endl;

	for (i = 1; i <= top; i++) 
	{  /* repeat for each level */
		int t = lua_type(L, i);
		switch (t) 
		{
		case LUA_TSTRING:
			std::cout <<lua_tostring(L, i);
			break;

		case LUA_TBOOLEAN:
			std::cout <<( lua_toboolean(L, i) ? "true" : "false");
			break;

		case LUA_TNUMBER:
			std::cout <<lua_tonumber(L, i);
			break;

		default:
			std::cout <<lua_typename(L, t);
			break;

		}
		std::cout <<"  ";
	}
	std::cout <<std::endl;
}
}

