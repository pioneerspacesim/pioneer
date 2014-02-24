// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Table.h"
#include "Lua.h"
#include "LuaConstants.h"
#include "LuaSignal.h"

namespace UI {

class LuaTable {
public:

	static int l_set_heading_row(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		UI::Context *c = t->GetContext();

		std::vector<UI::Widget*> widgets;

		if (lua_istable(l, 2)) {
			UI::Widget *w = UI::Lua::GetWidget(c, l, 2);
			if (w)
				widgets.push_back(w);
			else
				for (size_t i = 0; i < lua_rawlen(l, 2); i++) {
					lua_rawgeti(l, 2, i+1);
					widgets.push_back(UI::Lua::CheckWidget(c, l, -1));
					lua_pop(l, 1);
				}
		}
		else
			widgets.push_back(UI::Lua::CheckWidget(c, l, 2));

		t->SetHeadingRow(WidgetSet(widgets));

		lua_pushvalue(l, 1);
		return 1;
	}

	static void _add_row(Table *t, lua_State *l, int idx) {
		idx = lua_absindex(l, idx);

		UI::Context *c = t->GetContext();

		std::vector<UI::Widget*> widgets;

		if (lua_istable(l, idx)) {
			UI::Widget *w = UI::Lua::GetWidget(c, l, idx);
			if (w)
				widgets.push_back(w);
			else
				for (size_t i = 0; i < lua_rawlen(l, idx); i++) {
					lua_rawgeti(l, idx, i+1);
					widgets.push_back(UI::Lua::CheckWidget(c, l, -1));
					lua_pop(l, 1);
				}
		}
		else
			widgets.push_back(UI::Lua::CheckWidget(c, l, idx));

		t->AddRow(WidgetSet(widgets));
	}

	static int l_add_row(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		_add_row(t, l, 2);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_add_rows(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);

		luaL_checktype(l, 2, LUA_TTABLE);

		lua_pushnil(l);
		while (lua_next(l, 2)) {
			_add_row(t, l, -1);
			lua_pop(l, 1);
		}

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_clear_rows(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		t->ClearRows();
		return 0;
	}

	static int l_set_row_spacing(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		int spacing = luaL_checkinteger(l, 2);
		t->SetRowSpacing(spacing);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_column_spacing(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		int spacing = luaL_checkinteger(l, 2);
		t->SetColumnSpacing(spacing);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_row_alignment(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		UI::Table::RowAlignDirection dir = static_cast<UI::Table::RowAlignDirection>(LuaConstants::GetConstantFromArg(l, "UITableRowAlignDirection", 2));
		t->SetRowAlignment(dir);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_column_alignment(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		UI::Table::ColumnAlignDirection dir = static_cast<UI::Table::ColumnAlignDirection>(LuaConstants::GetConstantFromArg(l, "UITableColumnAlignDirection", 2));
		t->SetColumnAlignment(dir);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_heading_font(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		UI::Widget::Font font = static_cast<UI::Widget::Font>(LuaConstants::GetConstantFromArg(l, "UIFont", 2));
		t->SetHeadingFont(font);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_mouse_enabled(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		bool enabled = lua_toboolean(l, 2);
		t->SetMouseEnabled(enabled);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_attr_table_on_row_clicked(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		LuaSignal<unsigned int>().Wrap(l, t->onRowClicked);
		return 1;
	}

	static int l_scroll_to_top(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		t->SetScrollPosition(0.0f);
		return 0;
	}

	static int l_scroll_to_bottom(lua_State *l) {
		UI::Table *t = LuaObject<UI::Table>::CheckFromLua(1);
		t->SetScrollPosition(1.0f);
		return 0;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Table>::s_type = "UI.Table";

template <> void LuaObject<UI::Table>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "SetHeadingRow",      UI::LuaTable::l_set_heading_row      },
		{ "AddRow",             UI::LuaTable::l_add_row              },
		{ "AddRows",            UI::LuaTable::l_add_rows             },
		{ "ClearRows",          UI::LuaTable::l_clear_rows           },
		{ "SetRowSpacing",      UI::LuaTable::l_set_row_spacing      },
		{ "SetColumnSpacing",   UI::LuaTable::l_set_column_spacing   },
		{ "SetRowAlignment",    UI::LuaTable::l_set_row_alignment    },
		{ "SetColumnAlignment", UI::LuaTable::l_set_column_alignment },
		{ "SetHeadingFont",     UI::LuaTable::l_set_heading_font     },
		{ "SetMouseEnabled",    UI::LuaTable::l_set_mouse_enabled    },
		{ "ScrollToTop",        UI::LuaTable::l_scroll_to_top        },
		{ "ScrollToBottom",     UI::LuaTable::l_scroll_to_bottom     },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "onRowClicked",       UI::LuaTable::l_attr_table_on_row_clicked },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Table>::DynamicCastPromotionTest);
}
