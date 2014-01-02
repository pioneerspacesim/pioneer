// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NumberLabel.h"
#include "LuaObject.h"

namespace UI {

class LuaNumberLabel {
public:

	static int l_set_value(lua_State *l) {
		UI::NumberLabel *label = LuaObject<UI::NumberLabel>::CheckFromLua(1);
		double v = luaL_checknumber(l, 2);
		label->SetValue(v);
		return 0;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::NumberLabel>::s_type = "UI.NumberLabel";

template <> void LuaObject<UI::NumberLabel>::RegisterClass()
{
	static const char *l_parent = "UI.Label";

	static const luaL_Reg l_methods[] = {
		{ "SetValue", &LuaNumberLabel::l_set_value },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::NumberLabel>::DynamicCastPromotionTest);
}
