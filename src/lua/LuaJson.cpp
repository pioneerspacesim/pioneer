// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaJson.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "SaveGameManager.h"
#include "LuaObject.h"
#include "LuaUtils.h"

/*
 * Interface: Json
 */

// Do a simple JSON->Lua translation.
void LuaJson::PushToLua(lua_State *l, const Json &obj)
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
		for (size_t idx = 0; idx < size; idx++) {
			lua_pushinteger(l, idx + 1);
			PushToLua(l, obj[idx]);
			lua_settable(l, -3);
		}
	} break;
	case Json::value_t::object: {
		lua_newtable(l);
		for (const auto &pair : obj.items()) {
			lua_pushstring(l, pair.key().c_str());
			PushToLua(l, pair.value());
			lua_settable(l, -3);
		}
	} break;
	}
}

/*
 * Function: LoadJson
 *
 * Load a JSON file from the game's data sources, optionally applying all
 * files with the the name <filename>.patch as Json Merge Patch (RFC 7386) files
 *
 * > doc = Json.LoadJson(fileName)
 *
 * Parameters:
 *
 *   fileName - string
 *
 * Returns:
 *
 *   doc - table
 */
static int l_load_json(lua_State *l)
{
	std::string filename = luaL_checkstring(l, 1);

	Json data = JsonUtils::LoadJsonDataFile(filename);
	if (data.is_null())
		return luaL_error(l, "Error loading JSON file %s.", filename.c_str());

	LuaJson::PushToLua(l, data);

	return 1;
}

/*
 * Function: LoadSaveFile
 *
 * > gameDoc = Json.LoadSaveFile(fileName)
 *
 * Parameters:
 *
 *   fileName - string, File will be loaded from the 'savefiles' directory in the user's game directory.
 *
 * Returns:
 *
 *   gameDoc - table, corresponding to the json SaveGame document
 */
static int l_load_save_file(lua_State *l)
{
	std::string filename = luaL_checkstring(l, 1);

	Json data = SaveGameManager::LoadGameToJson(filename);
	if (data.is_null()) {
		return luaL_error(l, "Error loading JSON file %s.", filename.c_str());
	}

	LuaJson::PushToLua(l, data);

	return 1;
}

void LuaJson::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "LoadJson", l_load_json },
		{ "LoadSaveFile", l_load_save_file },
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
