// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Table.h"
#include "Lua.h"

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

	static int l_add_row(lua_State *l) {
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

		t->AddRow(WidgetSet(widgets));

		lua_pushvalue(l, 1);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Table>::s_type = "UI.Table";

template <> void LuaObject<UI::Table>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "SetHeadingRow", UI::LuaTable::l_set_heading_row },
		{ "AddRow",        UI::LuaTable::l_add_row         },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Table>::DynamicCastPromotionTest);
}
