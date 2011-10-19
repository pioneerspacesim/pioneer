#ifndef _LUACONSTANTS_H
#define _LUACONSTANTS_H

struct lua_State;

namespace LuaConstants {
	void Register(lua_State *l);

	int GetConstant(lua_State *l, const char *ns, const char *name);
	const char *GetConstantString(lua_State *l, const char *ns, int value);
}

#endif
