// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DropDown.h"
#include "LuaObject.h"
#include "LuaSignal.h"
namespace UI {

class LuaDropDown {
public:
	static int l_add_option(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		dropDown->AddOption(luaL_checkstring(l, 2));
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_selected_option(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		size_t len;
		const char *str = luaL_checklstring(l, 2, &len);
		const bool success = dropDown->SetSelectedOption(std::string(str, len));
		if (!success) {
			luaL_error(l, "UI.DropDown.SetSelectedOption: invalid option '%s' specified", str);
		}
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_selected_index(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		const int index = luaL_checkinteger(l, 2);
		if (index < 1 || size_t(index) > dropDown->NumItems()) {
			luaL_error(l, "UI.DropDown.SetSelectedIndex: invalid index %d specified", index);
		}
		dropDown->SetSelectedIndex(index - 1);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_attr_selected_option(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		if (dropDown->IsEmpty()) {
			lua_pushnil(l);
		} else {
			const std::string &selectedOption = dropDown->GetSelectedOption();
			lua_pushlstring(l, selectedOption.c_str(), selectedOption.size());
		}
		return 1;
	}

	static int l_attr_selected_index(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		if (dropDown->IsEmpty()) {
			lua_pushnil(l);
		} else {
			lua_pushinteger(l, dropDown->GetSelectedIndex() + 1);
		}
		return 1;
	}

	static int l_attr_on_option_selected(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		LuaSignal<unsigned int,const std::string &>().Wrap(l, dropDown->onOptionSelected);
		return 1;
	}
};

}

using namespace UI;

template <> const char *LuaObject<UI::DropDown>::s_type = "UI.DropDown";

template <> void LuaObject<UI::DropDown>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "AddOption",         LuaDropDown::l_add_option          },
		{ "SetSelectedOption", LuaDropDown::l_set_selected_option },
		{ "SetSelectedIndex",  LuaDropDown::l_set_selected_index  },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "selectedIndex",    LuaDropDown::l_attr_selected_index     },
		{ "selectedOption",   LuaDropDown::l_attr_selected_option    },
		{ "onOptionSelected", LuaDropDown::l_attr_on_option_selected },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::DropDown>::DynamicCastPromotionTest);
}
