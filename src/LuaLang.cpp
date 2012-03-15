#include "LuaLang.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "Lang.h"

/*
 * Interface: Lang
 *
 * Allows access to the dictionary.
 */

static void _build_dictionary_table(lua_State *l)
{
	typedef std::map<std::string, const char*> token_map;
	const token_map &tokens = Lang::GetDictionary();

	lua_createtable(l, 0, tokens.size());
	for (token_map::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
		lua_pushlstring(l, it->first.c_str(), it->first.size());
		lua_pushstring(l, it->second);
		lua_rawset(l, -3);
	}
}

/*
 * Function: GetDictionary
 *
 * Retrieve a Lua table for the current language.
 *
 * > dict = Lang.GetDictionary()
 * > print(dict['WE_HAVE_NO_BUSINESS_WITH_YOU'])
 *
 * Return:
 *
 *   dict - A Lua table mapping language token to translated string.
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   stable
 */
static int l_lang_get_dictionary(lua_State *l)
{
	LUA_DEBUG_START(l);
	lua_getfield(l, LUA_REGISTRYINDEX, "LangCoreDictionary");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		_build_dictionary_table(l);
		pi_lua_table_ro(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "LangCoreDictionary");
	}
	LUA_DEBUG_END(l, 1);
	return 1;
}

/*
 * Function: GetCoreLanguages
 *
 * Retrieve a Lua table listing the languages for which core translations are available.
 *
 * > langs = Lang.GetCoreLanguages()
 * > for k,v in ipairs(langs) do print('Core language: ' .. v); end
 *
 * Return:
 *
 *   langs - A Lua table (array form) listing core languages.
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   stable
 */
static int l_lang_get_core_languages(lua_State *l)
{
	LUA_DEBUG_START(l);
	const std::vector<std::string> &langs = Lang::GetAvailableLanguages();
	lua_createtable(l, langs.size(), 0);

	int i = 1;
	for (std::vector<std::string>::const_iterator
		it = langs.begin(); it != langs.end(); ++it) {

		lua_pushlstring(l, it->c_str(), it->size());
		lua_rawseti(l, -2, i);
		++i;
	}

	LUA_DEBUG_END(l, 1);
	return 1;
}

/*
 * Function: GetCurrentLanguage
 *
 * Retrieve the currently selected language.
 *
 * > lang = Lang.GetCurrentLanguage()
 * > print('Currently using ' .. lang .. ' language.')
 *
 * Return:
 *
 *   lang - The name of the currently selected language.
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   stable
 */
static int l_lang_get_current_language(lua_State *l)
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
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "GetDictionary",   l_lang_get_dictionary   },
		{ "GetCoreLanguages",    l_lang_get_core_languages     },
		{ "GetCurrentLanguage", l_lang_get_current_language },
		{ 0, 0 }
	};

	luaL_register(l, "Lang", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
