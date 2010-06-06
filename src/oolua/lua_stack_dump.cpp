#include "lua_includes.h"
#include "lua_stack_dump.h"
#include <iostream>
#include <string>

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
			{
				size_t len(0);
				char const* str = lua_tolstring(L,-1,&len);
				std::string value(std::string(str, len) );
				std::cout <<"LUA_TSTRING :" <<value;
			}
			break;

		case LUA_TBOOLEAN:
			std::cout <<"LUA_TBOOLEAN :" <<( lua_toboolean(L, i) ? "true" : "false");
			break;

		case LUA_TNUMBER:
			std::cout <<"LUA_TNUMBER :" <<lua_tonumber(L, i);
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

