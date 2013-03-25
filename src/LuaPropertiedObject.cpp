// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "PropertiedObject.h"

/*
 * Class: PropertiedObject
 *
 * A class with named properties. Any class that inherits from
 * <PropertiedObject> will have some number of attributes that are actually
 * properties. For the Lua programmer, the distinction is that properties can
 * be connected to, allowing functions to be called whenever the property
 * changes.
 *
 * The UI is a big user of this feature, see <Widget.Bind> for details.
 */

class LuaPropertiedObject {
public:

/*
 * Group: Methods
 */

/*
 * Method: Connect
 *
 * Register a function to be called when the named property changes.
 *
 * > object:Connect(property, function)
 *
 * Note that some properties can be changed often, sometimes every frame or
 * more. Do not use this mechanism for those properties; it will cause things
 * to slow down significantly. Consider another facility (eg a UI bind) to do
 * what you need.
 *
 * Example:
 *
 * > ship:Connect("fuel", function ()
 * >     print(string.format("player fuel %f%%", Game.player.fuel)
 * > end)
 *
 * Availability:
 *
 *   201308
 *
 * Status:
 *
 *   experimental
 */

	static void _signal_trampoline(PropertyMap &map, const std::string &k, LuaRef ref, lua_State *l) {
		ref.PushCopyToStack();
		pi_lua_protected_call(l, 0, 0);
	}

	static int l_connect(lua_State *l) {
		PropertiedObject *po = LuaObject<PropertiedObject>::CheckFromLua(1);
		const std::string propertyName(luaL_checkstring(l, 2));
		luaL_checktype(l, 3, LUA_TFUNCTION);
		LuaRef ref(l, 3);

		po->Properties().Connect(propertyName, sigc::bind(sigc::ptr_fun(_signal_trampoline), ref, l));

		return 0;
	}

};

template <> const char *LuaObject<PropertiedObject>::s_type = "PropertiedObject";

template <> void LuaObject<PropertiedObject>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "Connect", LuaPropertiedObject::l_connect },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
}
