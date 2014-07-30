// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BindingCapture.h"
#include "LuaObject.h"
#include "ui/LuaSignal.h"
#include "LuaPushPull.h"

inline void pi_lua_generic_push(lua_State * l, const KeyBindings::KeyBinding &value) {
	if (value.Enabled()) {
		pi_lua_generic_push(l, value.ToString());
	} else {
		lua_pushnil(l);
	}
}

inline void pi_lua_generic_push(lua_State * l, const KeyBindings::AxisBinding &value) {
	pi_lua_generic_push(l, value.ToString());
}

namespace GameUI {

class LuaKeyBindingCapture {
public:

	static int l_new(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<GameUI::KeyBindingCapture>::PushToLua(new KeyBindingCapture(c));
		return 1;
	}

	static int l_attr_binding(lua_State *l)
	{
		KeyBindingCapture *kbc = LuaObject<GameUI::KeyBindingCapture>::CheckFromLua(1);
		pi_lua_generic_push(l, kbc->GetBinding());
		return 1;
	}

	static int l_attr_binding_description(lua_State *l)
	{
		KeyBindingCapture *kbc = LuaObject<GameUI::KeyBindingCapture>::CheckFromLua(1);
		const KeyBindings::KeyBinding &kb = kbc->GetBinding();
		if (kb.Enabled()) {
			pi_lua_generic_push(l, kb.Description());
		} else {
			lua_pushnil(l);
		}
		return 1;
	}

	static int l_attr_on_capture(lua_State *l) {
		KeyBindingCapture *kbc = LuaObject<GameUI::KeyBindingCapture>::CheckFromLua(1);
		UI::LuaSignal<const KeyBindings::KeyBinding &>().Wrap(l, kbc->onCapture);
		return 1;
	}
};

class LuaAxisBindingCapture {
public:

	static int l_new(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<GameUI::AxisBindingCapture>::PushToLua(new AxisBindingCapture(c));
		return 1;
	}

	static int l_attr_binding(lua_State *l)
	{
		AxisBindingCapture *abc = LuaObject<GameUI::AxisBindingCapture>::CheckFromLua(1);
		pi_lua_generic_push(l, abc->GetBinding());
		return 1;
	}

	static int l_attr_binding_description(lua_State *l)
	{
		AxisBindingCapture *abc = LuaObject<GameUI::AxisBindingCapture>::CheckFromLua(1);
		pi_lua_generic_push(l, abc->GetBinding().Description());
		return 1;
	}

	static int l_attr_on_capture(lua_State *l) {
		AxisBindingCapture *abc = LuaObject<GameUI::AxisBindingCapture>::CheckFromLua(1);
		UI::LuaSignal<const KeyBindings::AxisBinding &>().Wrap(l, abc->onCapture);
		return 1;
	}
};

}

using namespace GameUI;

template <> const char *LuaObject<GameUI::KeyBindingCapture>::s_type = "UI.Game.KeyBindingCapture";
template <> void LuaObject<GameUI::KeyBindingCapture>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {
		{ "New",                LuaKeyBindingCapture::l_new },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "binding",            LuaKeyBindingCapture::l_attr_binding },
		{ "bindingDescription", LuaKeyBindingCapture::l_attr_binding_description },
		{ "onCapture",          LuaKeyBindingCapture::l_attr_on_capture },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<GameUI::KeyBindingCapture>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<GameUI::AxisBindingCapture>::s_type = "UI.Game.AxisBindingCapture";
template <> void LuaObject<GameUI::AxisBindingCapture>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {
		{ "New",                LuaAxisBindingCapture::l_new },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "binding",            LuaAxisBindingCapture::l_attr_binding },
		{ "bindingDescription", LuaAxisBindingCapture::l_attr_binding_description },
		{ "onCapture",          LuaAxisBindingCapture::l_attr_on_capture },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<GameUI::AxisBindingCapture>::DynamicCastPromotionTest);
}
