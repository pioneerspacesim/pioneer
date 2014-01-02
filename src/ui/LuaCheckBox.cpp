// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CheckBox.h"
#include "LuaObject.h"
#include "LuaSignal.h"

namespace UI {

class LuaCheckBox {
public:
	static int l_toggle(lua_State *l) {
		UI::CheckBox *c = LuaObject<UI::CheckBox>::CheckFromLua(1);
		c->Toggle();
		return 1;
	}

	static int l_set_state(lua_State *l) {
		UI::CheckBox *c = LuaObject<UI::CheckBox>::CheckFromLua(1);
		luaL_checktype(l, 2, LUA_TBOOLEAN);
		c->SetState(lua_toboolean(l, 2));
		return 1;
	}

	static int l_attr_is_checked(lua_State *l) {
		UI::CheckBox *c = LuaObject<UI::CheckBox>::CheckFromLua(1);
		lua_pushboolean(l, c->IsChecked());
		return 1;
	}

	static int l_attr_on_value_changed(lua_State *l) {
		UI::CheckBox *c = LuaObject<UI::CheckBox>::CheckFromLua(1);
		LuaSignal<bool>().Wrap(l, c->onValueChanged);
		return 1;
	}
};

}

using namespace UI;

template <> const char *LuaObject<UI::CheckBox>::s_type = "UI.CheckBox";

template <> void LuaObject<UI::CheckBox>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "Toggle", LuaCheckBox::l_toggle },
		{ "SetState", LuaCheckBox::l_set_state },
		{ 0, 0 }
	};
	static const luaL_Reg l_attrs[] = {
		{ "isChecked", LuaCheckBox::l_attr_is_checked },
		{ "onValueChanged", LuaCheckBox::l_attr_on_value_changed  },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::CheckBox>::DynamicCastPromotionTest);
}
