// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Adjustment.h"
#include "LuaObject.h"
#include "LuaSignal.h"
#include <iostream>
#include <iomanip>
#include <sstream>
namespace UI {

class LuaAdjustment {
public:
	static int l_set_inner_widget(lua_State *l) {
		UI::Adjustment *adj = LuaObject<UI::Adjustment>::CheckFromLua(1);
		UI::Widget *wid = LuaObject<UI::Widget>::CheckFromLua(2);;

		adj->SetInnerWidget(wid);
		return 1;
	}
	
	static int l_set_scroll_position(lua_State *l) {
		UI::Adjustment *adj = LuaObject<UI::Adjustment>::CheckFromLua(1);
		float value = luaL_checknumber(l, 2);
		
		Clamp(value, 0.0f, 1.0f);
// 		if (value > 1 || value < 0) {
// 			luaL_error(l, "LuaAdjustment:SetScrollPosition: %f  is out of range (0-1).", value);
// 			return 0;
// 		}
		
		adj->SetScrollPosition(value);
		return 1;
	}
	
	static int l_attr_on_change(lua_State *l) {
		UI::Adjustment *adj = LuaObject<UI::Adjustment>::CheckFromLua(1);
		LuaSignal<>().Wrap(l, adj->onSliderChanged);
		return 1;
	}
	
	static int l_attr_scroll_position(lua_State *l) {
		UI::Adjustment *adj = LuaObject<UI::Adjustment>::CheckFromLua(1);
		float pos = adj->GetScrollPosition();
		std::ostringstream ss;

		ss.precision(2);
		ss  << std::fixed << pos;
		
		
		lua_pushlstring(l, ss.str().c_str(), ss.str().size());
		return 1;
	}
	
	static int l_attr_inner_widget(lua_State *l) {
		UI::Adjustment *adj = LuaObject<UI::Adjustment>::CheckFromLua(1);
		UI::Widget *wid = adj->GetInnerWidget();
		LuaObject<UI::Widget>::PushToLua(wid);
		return 1;
	}
};

}

using namespace UI;

template <> const char *LuaObject<UI::Adjustment>::s_type = "UI.Adjustment";

template <> void LuaObject<UI::Adjustment>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "SetInnerWidget", LuaAdjustment::l_set_inner_widget },
		{ "SetScrollPosition", LuaAdjustment::l_set_scroll_position },
		
		{ 0, 0 }
	};
	static const luaL_Reg l_attrs[] = {
		{ "OnChange", LuaAdjustment::l_attr_on_change },
		{ "InnerWidget", LuaAdjustment::l_attr_inner_widget },
		{ "ScrollPosition", LuaAdjustment::l_attr_scroll_position },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Adjustment>::DynamicCastPromotionTest);
}
