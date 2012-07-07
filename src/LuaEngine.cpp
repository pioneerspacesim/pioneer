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
 *   stable
 */
static int l_engine_attr_userdir(lua_State *l)
{
	const std::string &userdir = FileSystem::GetUserDir();
	lua_pushlstring(l, userdir.c_str(), userdir.size());
	return 1;
}

void LuaEngine::Register()
{
	static const luaL_Reg l_attrs[] = {
		{ "rand",    l_engine_attr_rand    },
		{ "userdir", l_engine_attr_userdir },
		{ 0, 0 }
	};

	LuaObjectBase::CreateObject(0, l_attrs, 0);
	lua_setglobal(Pi::luaManager->GetLuaState(), "Engine");
}
