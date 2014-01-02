// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gauge.h"
#include "LuaObject.h"

namespace UI {

class LuaGauge {
public:

	static int l_set_value(lua_State *l) {
		UI::Gauge *gauge = LuaObject<UI::Gauge>::CheckFromLua(1);
		gauge->SetValue(luaL_checknumber(l, 2));
		return 0;
	}

	static int l_attr_value(lua_State *l) {
		UI::Gauge *gauge = LuaObject<UI::Gauge>::CheckFromLua(1);
		lua_pushnumber(l, gauge->GetValue());
		return 1;
	}

	static int l_set_upper_value(lua_State *l) {
		UI::Gauge *gauge = LuaObject<UI::Gauge>::CheckFromLua(1);
		gauge->SetUpperValue(luaL_checknumber(l, 2));
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_warning_level(lua_State *l) {
		UI::Gauge *gauge = LuaObject<UI::Gauge>::CheckFromLua(1);
		gauge->SetWarningLevel(luaL_checknumber(l, 2));
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_critical_level(lua_State *l) {
		UI::Gauge *gauge = LuaObject<UI::Gauge>::CheckFromLua(1);
		gauge->SetCriticalLevel(luaL_checknumber(l, 2));
		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_set_level_ascending(lua_State *l) {
		UI::Gauge *gauge = LuaObject<UI::Gauge>::CheckFromLua(1);
		gauge->SetLevelAscending(lua_toboolean(l, 2));
		lua_pushvalue(l, 1);
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Gauge>::s_type = "UI.Gauge";

template <> void LuaObject<UI::Gauge>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "SetValue",          &LuaGauge::l_set_value },
		{ "SetUpperValue",     &LuaGauge::l_set_upper_value },
		{ "SetWarningLevel",   &LuaGauge::l_set_warning_level },
		{ "SetCriticalLevel",  &LuaGauge::l_set_critical_level },
		{ "SetLevelAscending", &LuaGauge::l_set_level_ascending },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "value", &LuaGauge::l_attr_value },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Gauge>::DynamicCastPromotionTest);
}
