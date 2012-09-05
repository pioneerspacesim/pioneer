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
static int l_engine_attr_rand(lua_State *l)
{
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
 *   deprecated
 */
static int l_engine_attr_userdir(lua_State *l)
{
	const std::string &userdir = FileSystem::GetUserDir();
	lua_pushlstring(l, userdir.c_str(), userdir.size());
	return 1;
}

/*
 * Attribute: ticks
 *
 * Number of milliseconds since Pioneer was started. This should be used for
 * debugging purposes only (eg timing) and should never be used for game logic
 * of any kind.
 *
 * Availability:
 *
 *   alpha 26
 *
 * Status:
 *
 *   debug
 */
static int l_engine_attr_ticks(lua_State *l)
{
	lua_pushinteger(l, SDL_GetTicks());
	return 1;
}

void LuaEngine::Register()
{
	static const luaL_Reg l_attrs[] = {
		{ "rand",    l_engine_attr_rand    },
		{ "userdir", l_engine_attr_userdir },
		{ "ticks",   l_engine_attr_ticks   },
		{ 0, 0 }
	};

	LuaObjectBase::CreateObject(0, l_attrs, 0);
	lua_setglobal(Lua::manager->GetLuaState(), "Engine");
}
