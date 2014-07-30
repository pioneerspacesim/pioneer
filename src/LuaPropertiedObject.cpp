// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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


class PropertyConnection : public LuaWrappable {
public:
	PropertyConnection(const sigc::connection &conn) : m_connection(conn) {}
	void Disconnect() { m_connection.disconnect(); }
private:
	sigc::connection m_connection;
};

class LuaPropertyConnection {
public:

	static int l_disconnect(lua_State *l) {
		PropertyConnection *conn = LuaObject<PropertyConnection>::CheckFromLua(1);
		conn->Disconnect();
		return 0;
	}

};

template <> const char *LuaObject<PropertyConnection>::s_type = "PropertyConnection";

template <> void LuaObject<PropertyConnection>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "Disconnect", LuaPropertyConnection::l_disconnect },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
}



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
 * > connection = object:Connect(property, function)
 *
 * Note that some properties can be changed often, sometimes every frame or
 * more. Do not use this mechanism for those properties; it will cause things
 * to slow down significantly. Consider another facility (eg a UI bind) to do
 * what you need.
 *
 * Parameters:
 *
 *   property - name of the property to connect to
 *
 *   function - function to call when the property changes. This function is
 *              called with two arguments: the name of the property, and the
 *              new value.
 *
 * Returns:
 *
 *   connection - a object representing the property connection. If you are no
 *                longer interested in receiving notifications about changes
 *                to a property, call connection:Disconnect().
 *
 * Example:
 *
 * > ship:Connect("fuel", function (key, value)
 * >     print(string.format("player fuel %f%%", value)
 * > end)
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 */

	static void _signal_trampoline(PropertyMap &map, const std::string &k, LuaRef ref, lua_State *l) {
		ref.PushCopyToStack();
		lua_pushlstring(l, k.c_str(), k.size());
		map.PushLuaTable();
		lua_pushvalue(l, -2);
		lua_rawget(l, -2);
		lua_remove(l, -2);
		pi_lua_protected_call(l, 2, 0);
	}

	static int l_connect(lua_State *l) {
		PropertiedObject *po = LuaObject<PropertiedObject>::CheckFromLua(1);
		const std::string propertyName(luaL_checkstring(l, 2));
		luaL_checktype(l, 3, LUA_TFUNCTION);
		LuaRef ref(l, 3);

		sigc::connection conn = po->Properties().Connect(propertyName, sigc::bind(sigc::ptr_fun(_signal_trampoline), ref, l));

		LuaObject<PropertyConnection>::PushToLua(PropertyConnection(conn));
		return 1;
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

	LuaObject<PropertyConnection>::RegisterClass();
}
