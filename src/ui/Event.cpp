// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Event.h"
#include "LuaObject.h"
#include "LuaConstants.h"

namespace UI {

static void _settable(lua_State *l, const char *key, const Point &value)
{
	lua_pushstring(l, key);

	lua_newtable(l);
	pi_lua_settable(l, "x", value.x);
	pi_lua_settable(l, "y", value.y);

	lua_rawset(l, -3);
}

void KeyboardEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", LuaConstants::GetConstantString(l, "UIEventType", type));

	pi_lua_settable(l, "action", LuaConstants::GetConstantString(l, "UIKeyboardAction", action));

	// XXX expose sym and mod constants
}

void MouseButtonEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", LuaConstants::GetConstantString(l, "UIEventType", type));

	_settable(l, "pos", pos);

	pi_lua_settable(l, "action", LuaConstants::GetConstantString(l, "UIMouseButtonAction", action));
	pi_lua_settable(l, "button", LuaConstants::GetConstantString(l, "UIMouseButtonType", button));
}

void MouseMotionEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", LuaConstants::GetConstantString(l, "UIEventType", type));

	_settable(l, "pos", pos);
	_settable(l, "rel", rel);
}

void MouseWheelEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", LuaConstants::GetConstantString(l, "UIEventType", type));

	_settable(l, "pos", pos);

	pi_lua_settable(l, "direction", LuaConstants::GetConstantString(l, "UIMouseWheelDirection", direction));
}

}
