// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "List.h"
#include "LuaObject.h"
#include "LuaSignal.h"

namespace UI {

class LuaList {
public:

	static int l_add_option(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		list->AddOption(luaL_checkstring(l, 2));
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_attr_selected_option(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		const std::string &selectedOption(list->GetSelectedOption());
		lua_pushlstring(l, selectedOption.c_str(), selectedOption.size());
		return 1;
	}

	static int l_attr_on_option_selected(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		LuaSignal<unsigned int,const std::string &>().Wrap(l, list->onOptionSelected);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::List>::s_type = "UI.List";

template <> void LuaObject<UI::List>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "AddOption", LuaList::l_add_option },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "selectedOption",   LuaList::l_attr_selected_option    },
		{ "onOptionSelected", LuaList::l_attr_on_option_selected },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::List>::DynamicCastPromotionTest);
}
