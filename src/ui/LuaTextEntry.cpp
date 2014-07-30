// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextEntry.h"
#include "LuaObject.h"
#include "LuaSignal.h"
#include "LuaPushPull.h"

namespace UI {

class LuaTextEntry {
public:

	static int l_set_text(lua_State *l)
	{
		TextEntry *te = LuaObject<UI::TextEntry>::CheckFromLua(1);
		std::string new_text;
		pi_lua_generic_pull(l, 2, new_text);
		te->SetText(new_text);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_attr_text(lua_State *l)
	{
		TextEntry *te = LuaObject<UI::TextEntry>::CheckFromLua(1);
		const std::string &text(te->GetText());
		lua_pushlstring(l, text.c_str(), text.size());
		return 1;
	}

	static int l_attr_on_change(lua_State *l) {
		UI::TextEntry *te = LuaObject<UI::TextEntry>::CheckFromLua(1);
		LuaSignal<const std::string &>().Wrap(l, te->onChange);
		return 1;
	}

	static int l_attr_on_enter(lua_State *l) {
		UI::TextEntry *te = LuaObject<UI::TextEntry>::CheckFromLua(1);
		LuaSignal<const std::string &>().Wrap(l, te->onEnter);
		return 1;
	}
};

}

using namespace UI;

template <> const char *LuaObject<UI::TextEntry>::s_type = "UI.TextEntry";

template <> void LuaObject<UI::TextEntry>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "SetText",  LuaTextEntry::l_set_text },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "text",     LuaTextEntry::l_attr_text },
		{ "onChange", LuaTextEntry::l_attr_on_change },
		{ "onEnter",  LuaTextEntry::l_attr_on_enter },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::TextEntry>::DynamicCastPromotionTest);
}
