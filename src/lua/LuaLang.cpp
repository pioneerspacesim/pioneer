// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaLang.h"
#include "GameConfig.h"
#include "Lang.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "core/Log.h"
#include <algorithm>

// Generate an error message when an access is made to a translation that isn't in the table.
// This function is provided with two upvalues:
// - (1) resourceName
// - (2) langCode
static int _resource_index(lua_State *l)
{
	const char *token = lua_tostring(l, 2);
	const char *resourceName = lua_tostring(l, lua_upvalueindex(1));
	const char *langCode = lua_tostring(l, lua_upvalueindex(2));

	std::string tempStr = fmt::format("[NO_JSON]{}", token);
	lua_pushlstring(l, tempStr.c_str(), tempStr.size());

	// store the copy of the placeholder string in the translation table to speed up further access
	lua_pushvalue(l, 2);
	lua_pushvalue(l, -2);
	lua_rawset(l, 1);

	Log::Warning("Missing translation token '{}' in resource '{}/{}.json'.\n", token, resourceName, langCode);

	return 1;
}

static int _resource_newindex(lua_State *l)
{
	return luaL_error(l, "not allowed to add new translation tokens on-the-fly");
}

static int _get(lua_State *l)
{
	lua_rawget(l, -2);
	return 1;
}

static int l_lang_get_resource(lua_State *l)
{
	LUA_DEBUG_START(l);

	const std::string resourceName(luaL_checkstring(l, 1));
	const std::string langCode(lua_isnoneornil(l, 2) ? Lang::GetCore().GetLangCode() : lua_tostring(l, 2));

	const std::string key = resourceName + "_" + langCode;

	luaL_getsubtable(l, LUA_REGISTRYINDEX, "LangCache");

	// find it in the cache
	lua_getfield(l, -1, key.c_str());
	if (!lua_isnil(l, -1)) {
		lua_remove(l, -2);

		LUA_DEBUG_END(l, 1);
		return 1;
	}
	lua_pop(l, 1);

	lua_newtable(l);
	Lang::Resource &res = Lang::GetResource(resourceName, langCode);
	if (res.Load()) {
		for (auto i : res.GetStrings()) {
			const std::string token(i.first);
			const std::string text(i.second.empty() ? token : i.second);
			lua_pushlstring(l, text.c_str(), text.size());
			lua_setfield(l, -2, token.c_str());
		}
	} else {
		Log::Warning("Translation module {0} not found! This should be in data/lang/{0}/{1}.json. Returning dummy resource.\n",
			resourceName, langCode);
	}

	lua_pushcfunction(l, _get);
	lua_setfield(l, -2, "get");

	lua_newtable(l);
	// push the error message function
	lua_pushstring(l, resourceName.c_str());
	lua_pushstring(l, langCode.c_str());
	lua_pushcclosure(l, _resource_index, 2);
	lua_setfield(l, -2, "__index");

	lua_pushcfunction(l, _resource_newindex);
	lua_setfield(l, -2, "__newindex");
	lua_setmetatable(l, -2);

	// insert into cache
	lua_pushvalue(l, -1);
	lua_setfield(l, -3, key.c_str());
	lua_remove(l, -2);

	LUA_DEBUG_END(l, 1);
	return 1;
}

static int l_lang_get_available_languages(lua_State *l)
{
	LUA_DEBUG_START(l);
	const std::string resourceName(luaL_checkstring(l, 1));
	std::vector<std::string> langs = Lang::Resource::GetAvailableLanguages(resourceName);

	lua_createtable(l, langs.size(), 0);

	int i = 1;
	for (std::vector<std::string>::const_iterator
			 it = langs.begin();
		 it != langs.end(); ++it) {

		lua_pushlstring(l, it->c_str(), it->size());
		lua_rawseti(l, -2, i);
		++i;
	}

	LUA_DEBUG_END(l, 1);
	return 1;
}

static int l_lang_set_current_language(lua_State *l)
{
	const std::vector<std::string> langs = Lang::Resource::GetAvailableLanguages("core");
	std::string lang;
	pi_lua_generic_pull(l, 1, lang);
	if (std::find(langs.begin(), langs.end(), lang) == langs.end())
		return luaL_error(l, "The language '%s' is not known.", lang.c_str());
	Pi::config->SetString("Lang", lang);
	Pi::config->Save();
	// XXX change it!
	return 0;
}

static int l_lang_attr_current_language(lua_State *l)
{
	LUA_DEBUG_START(l);

	std::string lang = Pi::config->String("Lang");
	assert(!lang.empty()); // there should always be a Lang config value set

	lua_pushlstring(l, lang.c_str(), lang.size());

	LUA_DEBUG_END(l, 1);
	return 1;
}

void LuaLang::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "GetResource", l_lang_get_resource },
		{ "GetAvailableLanguages", l_lang_get_available_languages },
		{ "SetCurrentLanguage", l_lang_set_current_language },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "currentLanguage", l_lang_attr_current_language },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Lang");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
