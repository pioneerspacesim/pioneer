// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Grid.h"
#include "Lua.h"

namespace UI {

class LuaGrid {
public:

	static int l_set_row(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		UI::Context *c = g->GetContext();

		size_t rowNum = luaL_checkinteger(l, 2);
		luaL_checktype(l, 3, LUA_TTABLE);

		if (rowNum >= g->GetNumRows()) {
			luaL_error(l, "no such row %d (max is %d)", rowNum, g->GetNumRows()-1);
			return 0;
		}

		for (size_t i = 0; i < g->GetNumCols() && i < lua_rawlen(l, 3); i++) {
			lua_rawgeti(l, 3, i+1);
			if (lua_isnil(l, -1))
				g->ClearCell(i, rowNum);
			else
				g->SetCell(i, rowNum, UI::Lua::CheckWidget(c, l, -1));
			lua_pop(l, 1);
		}

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_column(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		UI::Context *c = g->GetContext();

		size_t colNum = luaL_checkinteger(l, 2);
		luaL_checktype(l, 3, LUA_TTABLE);

		if (colNum >= g->GetNumCols()) {
			luaL_error(l, "no such column %d (max is %d)", colNum, g->GetNumCols()-1);
			return 0;
		}

		for (size_t i = 0; i < g->GetNumRows() && i < lua_rawlen(l, 3); i++) {
			lua_rawgeti(l, 3, i+1);
			if (lua_isnil(l, -1))
				g->ClearCell(colNum, i);
			else
				g->SetCell(colNum, i, UI::Lua::CheckWidget(c, l, -1));
			lua_pop(l, 1);
		}

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_cell(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		UI::Context *c = g->GetContext();

		size_t colNum = luaL_checkinteger(l, 2);
		size_t rowNum = luaL_checkinteger(l, 3);
		UI::Widget *w = UI::Lua::CheckWidget(c, l, 4);

		if (colNum >= g->GetNumCols()) {
			luaL_error(l, "no such column %d (max is %d)", colNum, g->GetNumCols()-1);
			return 0;
		}
		if (rowNum >= g->GetNumRows()) {
			luaL_error(l, "no such row %d (max is %d)", rowNum, g->GetNumRows()-1);
			return 0;
		}

		g->SetCell(colNum, rowNum, w);

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_clear_row(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		g->ClearRow(luaL_checkinteger(l, 2));
		return 0;
	}

	static int l_clear_column(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		g->ClearColumn(luaL_checkinteger(l, 2));
		return 0;
	}

	static int l_clear_cell(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		unsigned int colNum = luaL_checkinteger(l, 2);
		unsigned int rowNum = luaL_checkinteger(l, 3);
		g->ClearCell(colNum, rowNum);
		return 0;
	}

	static int l_clear(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		g->Clear();
		return 0;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Grid>::s_type = "UI.Grid";

template <> void LuaObject<UI::Grid>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "SetRow",      LuaGrid::l_set_row      },
		{ "SetColumn",   LuaGrid::l_set_column   },
		{ "SetCell",     LuaGrid::l_set_cell     },

		{ "ClearRow",    LuaGrid::l_clear_row    },
		{ "ClearColumn", LuaGrid::l_clear_column },
		{ "ClearCell",   LuaGrid::l_clear_cell   },
		{ "Clear",       LuaGrid::l_clear        },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Grid>::DynamicCastPromotionTest);
}
