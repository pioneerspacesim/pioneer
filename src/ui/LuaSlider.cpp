// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Slider.h"
#include "LuaObject.h"
#include "LuaSignal.h"

namespace UI {

class LuaSlider {
public:

	static int l_set_range(lua_State *l) {
		UI::Slider *slider = LuaObject<UI::Slider>::CheckFromLua(1);
		const float min = luaL_checknumber(l, 2);
		const float max = luaL_checknumber(l, 3);
		if (min >= max) {
			return luaL_error(l, "Slider range minimum must be strictly less than the maximum");
		}
		slider->SetRange(min, max);
		lua_settop(l, 1);
		return 1;
	}

	static int l_get_range(lua_State *l) {
		UI::Slider *slider = LuaObject<UI::Slider>::CheckFromLua(1);
		float min, max;
		slider->GetRange(min, max);
		lua_pushnumber(l, min);
		lua_pushnumber(l, max);
		return 2;
	}

	static int l_set_value(lua_State *l) {
		UI::Slider *slider = LuaObject<UI::Slider>::CheckFromLua(1);
		const float v = luaL_checknumber(l, 2);
		slider->SetValue(v);
		lua_settop(l, 1);
		return 1;
	}

	static int l_get_value(lua_State *l) {
		UI::Slider *slider = LuaObject<UI::Slider>::CheckFromLua(1);
		lua_pushnumber(l, slider->GetValue());
		return 1;
	}

	static int l_attr_on_value_changed(lua_State *l) {
		UI::Slider *slider = LuaObject<UI::Slider>::CheckFromLua(1);
		LuaSignal<float>().Wrap(l, slider->onValueChanged);
		return 1;
	}

};

class LuaHSlider;
class LuaVSlider;

}

using namespace UI;

template <> const char *LuaObject<UI::Slider>::s_type = "UI.Slider";

template <> void LuaObject<UI::Slider>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "SetRange", LuaSlider::l_set_range },
		{ "GetRange", LuaSlider::l_get_range },
		{ "SetValue", LuaSlider::l_set_value },
		{ "GetValue", LuaSlider::l_get_value },

		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "onValueChanged", LuaSlider::l_attr_on_value_changed  },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Slider>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::HSlider>::s_type = "UI.HSlider";

template <> void LuaObject<UI::HSlider>::RegisterClass()
{
	static const char *l_parent = "UI.Slider";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::HSlider>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::VSlider>::s_type = "UI.VSlider";

template <> void LuaObject<UI::VSlider>::RegisterClass()
{
	static const char *l_parent = "UI.Slider";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::VSlider>::DynamicCastPromotionTest);
}
