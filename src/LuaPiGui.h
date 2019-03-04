// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPIGUI_H
#define _LUAPIGUI_H
#include "LuaObject.h"
#include "LuaPushPull.h"
#include <tuple>

#include "vector2.h"

struct TScreenSpace
{
	TScreenSpace(const bool onScreen, const vector2d &screenPos, const vector3d &direction) : _onScreen(onScreen), _screenPosition(screenPos), _direction(direction) {}
	bool _onScreen;
	vector2d _screenPosition;
	vector3d _direction;
};

void pi_lua_generic_push(lua_State *l, const vector3d &v);
void pi_lua_generic_push(lua_State *l, const vector3f &v);
void pi_lua_generic_pull(lua_State *l, int index, vector3d &vector);
int pushOnScreenPositionDirection(lua_State *l, vector3d position);
TScreenSpace lua_world_space_to_screen_space(const vector3d &pos);
#endif
