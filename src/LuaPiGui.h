// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPIGUI_H
#define _LUAPIGUI_H
#include "LuaObject.h"
#include "LuaPushPull.h"
#include <tuple>

#include "vector2.h"

class Body;

bool first_body_is_more_important_than(Body* body, Body* other);

struct TScreenSpace
{
	TScreenSpace(const bool onScreen, const vector2d &screenPos, const vector3d &direction) : _onScreen(onScreen), _screenPosition(screenPos), _direction(direction) {}
	bool _onScreen;
	vector2d _screenPosition;
	vector3d _direction;
	Body *_body;
};

typedef std::vector<TScreenSpace> TSS_vector;

int pushOnScreenPositionDirection(lua_State *l, vector3d position);
TScreenSpace lua_world_space_to_screen_space(const vector3d &pos);
#endif
