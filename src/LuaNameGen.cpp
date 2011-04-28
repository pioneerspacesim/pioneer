#include "LuaNameGen.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaRand.h"
#include "NameGenerator.h"
#include "Pi.h"

static int l_namegen_full_name(lua_State *l)
{
	MTRand *rand;
	bool female = false;
	if ((rand = LuaRand::CheckFromLua(1)))
		female = lua_toboolean(l, 2);
	else {
		rand = &Pi::rng;
		female = lua_toboolean(l, 1);
	}

	lua_pushstring(l, NameGenerator::FullName(*rand, female).c_str());
	return 1;
}

static int l_namegen_surname(lua_State *l)
{
	MTRand *rand;
	if (!(rand = LuaRand::CheckFromLua(1)))
		rand = &Pi::rng;

	lua_pushstring(l, NameGenerator::Surname(*rand).c_str());
	return 1;
}
	
static int l_namegen_planet_name(lua_State *l)
{
	MTRand *rand;
	if (!(rand = LuaRand::CheckFromLua(1)))
		rand = &Pi::rng;

	lua_pushstring(l, NameGenerator::PlanetName(*rand).c_str());
	return 1;
}

void LuaNameGen::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "FullName",   l_namegen_full_name   },
		{ "Surname",    l_namegen_surname     },
		{ "PlanetName", l_namegen_planet_name },
		{ 0, 0 }
	};

	luaL_register(l, "NameGen", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
