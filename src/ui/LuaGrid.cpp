#include "Grid.h"
#include "LuaObject.h"

namespace UI {

class LuaGrid {
public:

	static int l_set_row(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		int rowNum = luaL_checkinteger(l, 2);
		luaL_checktype(l, 3, LUA_TTABLE);

		Widget *widgets[8];
		for (size_t i = 0; i < 8 && i < lua_rawlen(l, 3); i++) {
			lua_rawgeti(l, 3, i+1);
			widgets[i] = LuaObject<UI::Widget>::CheckFromLua(-1);
			lua_pop(l, 1);
		}

		g->SetRow(rowNum, WidgetSet(widgets, lua_rawlen(l, 3)));
		return 0;
	}

	static int l_set_column(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		int colNum = luaL_checkinteger(l, 2);
		luaL_checktype(l, 3, LUA_TTABLE);

		Widget *widgets[8];
		for (size_t i = 0; i < 8 && i < lua_rawlen(l, 3); i++) {
			lua_rawgeti(l, 3, i+1);
			widgets[i] = LuaObject<UI::Widget>::CheckFromLua(-1);
			lua_pop(l, 1);
		}

		g->SetColumn(colNum, WidgetSet(widgets, lua_rawlen(l, 3)));
		return 0;
	}

	static int l_set_cell(lua_State *l) {
		UI::Grid *g = LuaObject<UI::Grid>::CheckFromLua(1);
		int colNum = luaL_checkinteger(l, 2);
		int rowNum = luaL_checkinteger(l, 3);
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(4);
		g->SetCell(colNum, rowNum, w);
		return 0;
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
		{ "Clear",       LuaGrid::l_clear        },
        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Grid>::DynamicCastPromotionTest);
}
