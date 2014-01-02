// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUACONSTANTS_H
#define _LUACONSTANTS_H

struct lua_State;

namespace LuaConstants {
	void Register(lua_State *l);

	int GetConstantFromArg(lua_State *l, const char *ns, int idx);
	int GetConstant(lua_State *l, const char *ns, const char *name);
}

#endif
