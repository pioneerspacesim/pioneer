// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPIGUI_H
#define _LUAPIGUI_H
#include "LuaObject.h"
#include "LuaPushPull.h"
#include <tuple>

void pi_lua_generic_push(lua_State *l, const vector3d &v);
void pi_lua_generic_pull(lua_State *l, int index, vector3d &vector);
int pushOnScreenPositionDirection(lua_State *l, vector3d position);
std::tuple<bool, vector3d, vector3d> lua_world_space_to_screen_space(vector3d pos);
#endif

