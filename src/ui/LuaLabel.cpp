// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Label.h"
#include "LuaObject.h"

namespace UI {

class LuaLabel {
public:

	static int l_set_text(lua_State *l) {
		UI::Label *label = LuaObject<UI::Label>::CheckFromLua(1);
		const std::string text(luaL_checkstring(l, 2));
		label->SetText(text);
		return 0;
	}

	static int l_set_color(lua_State *l) {
		UI::Label *label = LuaObject<UI::Label>::CheckFromLua(1);
		Color c = Color::FromLuaTable(l, 2);
		label->SetColor(c);
		lua_pushvalue(l, 1);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Label>::s_type = "UI.Label";

template <> void LuaObject<UI::Label>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "SetText", &LuaLabel::l_set_text },
		{ "SetColor", &LuaLabel::l_set_color },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Label>::DynamicCastPromotionTest);
}
