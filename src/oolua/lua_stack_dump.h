#ifndef LUA_DEBUG_STACK_DUMP_H_
#	define LUA_DEBUG_STACK_DUMP_H_

struct lua_State;

namespace OOLUA
{
	void lua_stack_dump(lua_State * const L);
}

#endif//LUA_DEBUG_STACK_DUMP_H_


