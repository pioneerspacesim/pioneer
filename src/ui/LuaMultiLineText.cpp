// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MultiLineText.h"
#include "LuaObject.h"

namespace UI {

class LuaMultiLineText {
public:

	static int l_set_text(lua_State *l) {
		UI::MultiLineText *mlt = LuaObject<UI::MultiLineText>::CheckFromLua(1);
		const std::string text(luaL_checkstring(l, 2));
		mlt->SetText(text);
		return 0;
	}

	static int l_append_text(lua_State *l) {
		UI::MultiLineText *mlt = LuaObject<UI::MultiLineText>::CheckFromLua(1);
		const std::string text(luaL_checkstring(l, 2));
		mlt->AppendText(text);
		return 0;
	}
};

}

using namespace UI;

template <> const char *LuaObject<UI::MultiLineText>::s_type = "UI.MultiLineText";

template <> void LuaObject<UI::MultiLineText>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "SetText",    &LuaMultiLineText::l_set_text    },
		{ "AppendText", &LuaMultiLineText::l_append_text },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::MultiLineText>::DynamicCastPromotionTest);
}
