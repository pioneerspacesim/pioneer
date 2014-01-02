// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

	static int l_set_selected_option(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		size_t len;
		const char *str = luaL_checklstring(l, 2, &len);
		const bool success = list->SetSelectedOption(std::string(str, len));
		if (!success) {
			luaL_error(l, "UI.List.SetSelectedOption: invalid option '%s' specified", str);
		}
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_selected_index(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		const int index = luaL_checkinteger(l, 2);
		if (index < 1 || size_t(index) > list->NumItems()) {
			luaL_error(l, "UI.List.SetSelectedIndex: invalid index %d specified", index);
		}
		list->SetSelectedIndex(index - 1);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_attr_selected_option(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		if (list->IsEmpty()) {
			lua_pushnil(l);
		} else {
			const std::string &selectedOption = list->GetSelectedOption();
			lua_pushlstring(l, selectedOption.c_str(), selectedOption.size());
		}
		return 1;
	}

	static int l_attr_selected_index(lua_State *l) {
		UI::List *list = LuaObject<UI::List>::CheckFromLua(1);
		if (list->IsEmpty()) {
			lua_pushnil(l);
		} else {
			lua_pushinteger(l, list->GetSelectedIndex() + 1);
		}
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
		{ "AddOption",         LuaList::l_add_option          },
		{ "SetSelectedOption", LuaList::l_set_selected_option },
		{ "SetSelectedIndex",  LuaList::l_set_selected_index  },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "selectedIndex",    LuaList::l_attr_selected_index     },
		{ "selectedOption",   LuaList::l_attr_selected_option    },
		{ "onOptionSelected", LuaList::l_attr_on_option_selected },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::List>::DynamicCastPromotionTest);
}
