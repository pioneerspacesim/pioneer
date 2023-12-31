// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPIGUI_H
#define _LUAPIGUI_H

#include "LuaObject.h"
#include "LuaPushPull.h"
#include "LuaTable.h"

#include "vector2.h"
#include "vector3.h"

class Body;
struct ImGuiStyle;
struct ImVec2;
struct ImColor;

void pi_lua_generic_push(lua_State *l, const ImVec2 &vec);
void pi_lua_generic_pull(lua_State *l, int index, ImVec2 &vec);
void pi_lua_generic_pull(lua_State *l, int index, ImColor &color);

namespace PiGui {
	bool first_body_is_more_important_than(Body *body, Body *other);

	struct TScreenSpace {
		TScreenSpace(const bool onScreen, const vector2d &screenPos, const vector3d &direction) :
			_onScreen(onScreen), _screenPosition(screenPos), _direction(direction) {}
		bool _onScreen;
		vector2d _screenPosition;
		vector3d _direction;
		Body *_body;
	};

	typedef std::vector<TScreenSpace> TSS_vector;

	int pushOnScreenPositionDirection(lua_State *l, vector3d position);

	void load_theme_from_table(LuaTable &table, ImGuiStyle &style);
} // namespace PiGui

#endif
