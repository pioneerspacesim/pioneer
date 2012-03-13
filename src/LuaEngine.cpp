#include "LuaEngine.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaRand.h"
#include "Pi.h"
#include "utils.h"
#include "FileSystem.h"

/*
 * Interface: Engine
 *
 * A global table that exposes a number of non-game-specific values from the
 * game engine.
 *
 */

static int l_engine_meta_index(lua_State *l)
{
	const char *key = luaL_checkstring(l, 2);

	/*
	 * Attribute: rand
	 *
	 * The global <Rand> object. Its stream of values will be different across
     * multiple Pioneer runs. Use this when you just need a random number and
     * don't care about the seed.
	 *
	 * Availability:
	 *
	 *  alpha 10
	 *
	 * Status:
	 *
	 *  stable
	 */
	if (strcmp(key, "rand") == 0) {
		LuaRand::PushToLua(&Pi::rng);
		return 1;
	}

	/*
	 * Attribute: userdir
	 *
	 * The Pioneer configuration directory (should be writable).
	 *
	 * Availability:
	 *
	 *   alpha 14
	 *
	 * Status:
	 *
	 *   stable
	 */
	if (strcmp(key, "userdir") == 0) {
		const std::string &userdir = FileSystem::GetUserDir();
		lua_pushlstring(l, userdir.c_str(), userdir.size());
		return 1;
	}

	lua_pushnil(l);
	return 1;
}

void LuaEngine::Register()
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);
    pi_lua_table_ro(l);

	luaL_newmetatable(l, "Engine");
	
	lua_pushstring(l, "__index");
	lua_pushcfunction(l, l_engine_meta_index);
	lua_rawset(l, -3);

	lua_setmetatable(l, -2);

	lua_setfield(l, LUA_GLOBALSINDEX, "Engine");
	
	LUA_DEBUG_END(l, 0);
}
