// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPIGUI_H
#define _LUAPIGUI_H
#include "LuaObject.h"

bool luaL_checkbool(lua_State *l, int index);
vector3d luaL_checkvector3d(lua_State *l, int index);
#endif
