// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Scroller.h"
#include "Lua.h"

namespace UI {

class LuaScroller {
public:

	static int l_set_inner_widget(lua_State *l) {
		Scroller *s = LuaObject<UI::Scroller>::CheckFromLua(1);
		Context *c = s->GetContext();
		Widget *w = UI::Lua::CheckWidget(c, l, 2);
		s->SetInnerWidget(w);
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_remove_inner_widget(lua_State *l) {
		Scroller *s = LuaObject<UI::Scroller>::CheckFromLua(1);
		s->RemoveInnerWidget();
		return 0;
	}

	static int l_attr_inner_widget(lua_State *l) {
		Scroller *s = LuaObject<UI::Scroller>::CheckFromLua(1);
		LuaObject<UI::Widget>::PushToLua(s->GetInnerWidget());
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Scroller>::s_type = "UI.Scroller";

template <> void LuaObject<UI::Scroller>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "SetInnerWidget",    LuaScroller::l_set_inner_widget    },
		{ "RemoveInnerWidget", LuaScroller::l_remove_inner_widget },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "innerWidget", LuaScroller::l_attr_inner_widget },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Scroller>::DynamicCastPromotionTest);
}
