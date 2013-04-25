// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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
	
	static int l_set_option(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		std::string option = luaL_checkstring(l, 2);
		bool success = dropDown->SetOption(option);
		if(success) {
			lua_pushvalue(l, 1);
		}
		else {
			luaL_error(l, "no such video mode %s", option.c_str());
			return 0;
		}
		return 1;
	}
	
	static int l_attr_selected_option(lua_State *l) {
		UI::DropDown *dropDown = LuaObject<UI::DropDown>::CheckFromLua(1);
		const std::string &selectedOption(dropDown->GetSelectedOption());
		lua_pushlstring(l, selectedOption.c_str(), selectedOption.size());
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
		{ "AddOption", LuaDropDown::l_add_option },
		{ "SetOption", LuaDropDown::l_set_option },
		{ 0, 0 }
	};
	static const luaL_Reg l_attrs[] = {
		{ "selectedOption",   LuaDropDown::l_attr_selected_option    },
		{ "onOptionSelected", LuaDropDown::l_attr_on_option_selected },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::DropDown>::DynamicCastPromotionTest);
}
