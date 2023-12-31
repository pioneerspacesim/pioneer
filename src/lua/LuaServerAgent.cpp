// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifdef ENABLE_SERVER_AGENT

#include "LuaServerAgent.h"
#include "LuaObject.h"
#include "LuaRef.h"
#include "Pi.h"
#include "ServerAgent.h"
#include <json/json.h>

/*
 * Interface: ServerAgent
 *
 * An object that provides access to an RPC server. This allows your scripts
 * to call remote functions and send and recieve data from a remote server.
 *
 * This page documents the API only. There are other things that need to be
 * done to enable and use this interface. See
 * https://wiki.pioneerspacesim.net/wiki/ServerAgent for more information.
 */

struct CallbackPair {
	CallbackPair(lua_State *l, int successIndex, int failIndex) :
		lua(l),
		successCallback(l, successIndex),
		failCallback(l, failIndex)
	{}
	lua_State *lua;
	LuaRef successCallback;
	LuaRef failCallback;
};

static Json _lua_to_json(lua_State *l, int idx)
{
	int data = lua_absindex(l, idx);

	switch (lua_type(l, data)) {
	case LUA_TNIL:
		return Json();

	case LUA_TNUMBER:
		return Json(lua_tonumber(l, data));

	case LUA_TBOOLEAN:
		return Json(lua_toboolean(l, data));

	case LUA_TSTRING:
		return Json(lua_tostring(l, data));

	case LUA_TTABLE: {

		// XXX handle arrays

		Json object(Json::objectValue);

		lua_pushnil(l);
		while (lua_next(l, data)) {
			const std::string key(luaL_checkstring(l, -2));
			Json value(_lua_to_json(l, -1));
			object[key] = value;
			lua_pop(l, 1);
		}

		return object;
	}

	default:
		luaL_error(l, "can't convert Lua type %s to JSON", lua_typename(l, lua_type(l, idx)));
		return Json();
	}

	// shouldn't get here
	assert(0);

	return Json();
}

static void _json_to_lua(lua_State *l, const Json &data)
{
	LUA_DEBUG_START(l);

	switch (data.type()) {
	case Json::nullValue:
		lua_pushnil(l);
		break;

	case Json::intValue:
	case Json::uintValue:
	case Json::realValue:
		lua_pushnumber(l, data.asDouble());
		break;

	case Json::stringValue: {
		const std::string &str(data.asString());
		lua_pushlstring(l, str.c_str(), str.size());
		break;
	}

	case Json::booleanValue:
		lua_pushboolean(l, data.asBool());
		break;

	case Json::arrayValue: {
		lua_newtable(l);
		for (int i = 0; i < int(data.size()); i++) {
			lua_pushinteger(l, i + 1);
			_json_to_lua(l, data[i]);
			lua_rawset(l, -3);
		}
		break;
	}

	case Json::objectValue: {
		lua_newtable(l);
		for (Json::const_iterator i = data.begin(); i != data.end(); ++i) {
			const std::string &key(i.key().asString());
			lua_pushlstring(l, key.c_str(), key.size());
			_json_to_lua(l, *i);
			lua_rawset(l, -3);
		}
		break;
	}
	}

	LUA_DEBUG_END(l, 1);
}

static void _success_callback(const Json &data, void *userdata)
{
	CallbackPair *cp = reinterpret_cast<CallbackPair *>(userdata);
	if (!cp->successCallback.IsValid()) return;

	cp->successCallback.PushCopyToStack();

	_json_to_lua(cp->lua, data);
	pi_lua_protected_call(cp->lua, 1, 0);

	delete cp;
}

static void _fail_callback(const std::string &error, void *userdata)
{
	CallbackPair *cp = reinterpret_cast<CallbackPair *>(userdata);
	if (!cp->failCallback.IsValid()) return;

	cp->failCallback.PushCopyToStack();

	lua_pushlstring(cp->lua, error.c_str(), error.size());
	pi_lua_protected_call(cp->lua, 1, 0);

	delete cp;
}

/*
 * Function: Call
 *
 * Call a remote function, optionally sending or receiving data.
 *
 * > ServerAgent.Call(method, data, onSuccess, onFailure)
 *
 * Parameters:
 *
 *   method - name of the remote function/method to call.
 *
 *   data - optional. an arbitrary data item to be sent to the server along
 *          with the call.
 *
 *   onSuccess - optional. a function to call when the remote call completes
 *               successfully. If the call returned data, it will be passed as
 *               the first argument to the function.
 *
 *   onFailure - optional. a function to call if the remote call fails for any
 *               reason. The text of the error will be passed as the first
 *               argument to the function.
 *
 * Availability:
 *
 *   alpha 29
 *
 * Status:
 *
 *   experimental
 */
static int l_serveragent_call(lua_State *l)
{
	const std::string method(luaL_checkstring(l, 1));
	const Json data(_lua_to_json(l, 2));

	int successIndex = LUA_NOREF, failIndex = LUA_NOREF;

	if (lua_gettop(l) > 2) {
		luaL_checktype(l, 3, LUA_TFUNCTION);
		successIndex = 3;
	}
	if (lua_gettop(l) > 3) {
		luaL_checktype(l, 4, LUA_TFUNCTION);
		failIndex = 4;
	}

	CallbackPair *cp = new CallbackPair(l, successIndex, failIndex);

	Pi::serverAgent->Call(method, data, sigc::ptr_fun(_success_callback), sigc::ptr_fun(_fail_callback), cp);

	return 0;
}

void LuaServerAgent::Register()
{
	static const luaL_Reg l_methods[] = {
		{ "Call", l_serveragent_call },
		{ 0, 0 }
	};

	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setglobal(Lua::manager->GetLuaState(), "ServerAgent");
}

#endif
