// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaJson.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"

// Do a simple JSON->Lua translation.
static void _push_json_to_lua(lua_State *l, Json &obj)
{
	lua_checkstack(l, 20);

	switch (obj.type()) {
	case Json::value_t::string: {
		lua_pushstring(l, obj.get<std::string>().c_str());
	} break;
	case Json::value_t::boolean: {
		lua_pushboolean(l, obj.get<bool>());
	} break;
	case Json::value_t::null:
	default: {
		lua_pushnil(l);
	} break;
	case Json::value_t::number_integer:
	case Json::value_t::number_unsigned: {
		lua_pushinteger(l, obj);
	} break;
	case Json::value_t::number_float: {
		lua_pushnumber(l, obj);
	} break;
	case Json::value_t::array: {
		lua_newtable(l);
		size_t size = obj.size();
		lua_pushinteger(l, size);
		lua_setfield(l, -2, "n");
		for (size_t idx = 0; idx < size; idx++) {
			lua_pushinteger(l, idx);
			_push_json_to_lua(l, obj[idx]);
			lua_settable(l, -3);
		}
	} break;
	case Json::value_t::object: {
		lua_newtable(l);
		for (Json::iterator it = obj.begin(); it != obj.end(); it++) {
			lua_pushstring(l, it.key().c_str());
			_push_json_to_lua(l, it.value());
			lua_settable(l, -3);
		}
	} break;
	}
}

static int l_load_json(lua_State *l)
{
	std::string filename = luaL_checkstring(l, 1);

	Json data = JsonUtils::LoadJsonDataFile(filename);
	if (data.is_null())
		return luaL_error(l, "Error loading JSON file %s.", filename.c_str());

	_push_json_to_lua(l, data);

	return 1;
}

void LuaJson::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "LoadJson", l_load_json },
		{ NULL, NULL }
	};

	static const luaL_Reg l_attrs[] = {
		{ NULL, NULL }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Json");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
