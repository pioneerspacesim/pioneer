// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SelectorBox.h"
#include "LuaObject.h"

namespace UI {

class LuaSelectorBox {
public:

	static int l_set_color(lua_State *l) {
		UI::SelectorBox *sbox = LuaObject<UI::SelectorBox>::CheckFromLua(1);
		const float r = luaL_checknumber(l, 2);
		const float g = luaL_checknumber(l, 3);
		const float b = luaL_checknumber(l, 4);
		const float a = luaL_optnumber(l, 5, 1.0);
		sbox->SetColor(Color(255*r, 255*g, 255*b, 255*a));
		lua_settop(l, 1);
		return 1;
	}

	static int l_set_shown(lua_State *l) {
		UI::SelectorBox *sbox = LuaObject<UI::SelectorBox>::CheckFromLua(1);
		sbox->SetShown(lua_toboolean(l, 2));
		lua_settop(l, 1);
		return 1;
	}

	static int l_attr_shown(lua_State *l) {
		UI::SelectorBox *sbox = LuaObject<UI::SelectorBox>::CheckFromLua(1);
		lua_pushboolean(l, sbox->IsShown());
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::SelectorBox>::s_type = "UI.SelectorBox";

template <> void LuaObject<UI::SelectorBox>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {
		{ "SetColor", LuaSelectorBox::l_set_color },
		{ "SetShown", LuaSelectorBox::l_set_shown },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "shown", LuaSelectorBox::l_attr_shown },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::SelectorBox>::DynamicCastPromotionTest);
}
