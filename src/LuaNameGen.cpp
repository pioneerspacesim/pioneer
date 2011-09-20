#include "LuaNameGen.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaRand.h"
#include "NameGenerator.h"
#include "Pi.h"

/*
 * Interface: NameGen
 *
 * Functions for generating names.
 */

/*
 * Function: FullName
 *
 * Create a full name (first + surname) string
 *
 * > name = Namegen.FullName(isfemale, rand)
 *
 * Parameters:
 *
 *   isfemale - whether to generate a male or female name. true for female,
 *              false for male
 *
 *   rand - optional, the <Rand> object to use to generate the name. if
 *          omitted, <Engine.rand> will be used
 *
 * Return:
 *
 *   name - a string containing the name
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_namegen_full_name(lua_State *l)
{
	bool female = lua_toboolean(l, 1);

	MTRand *rand = LuaRand::CheckFromLua(2);
	if (!rand) rand = &Pi::rng;

	lua_pushstring(l, NameGenerator::FullName(*rand, female).c_str());
	return 1;
}

/*
 * Function: Surname
 *
 * Create a surname string
 *
 * > name = Namegen.Surname(rand)
 *
 * Parameters:
 *
 *   rand - optional, the <Rand> object to use to generate the name. if
 *          omitted, <Engine.rand> will be used
 *
 * Return:
 *
 *   name - a string containing the name
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_namegen_surname(lua_State *l)
{
	MTRand *rand;
	if (!(rand = LuaRand::CheckFromLua(1)))
		rand = &Pi::rng;

	lua_pushstring(l, NameGenerator::Surname(*rand).c_str());
	return 1;
}
	
/*
 * Function: PlanetName
 *
 * Create a planet name
 *
 * > name = Namegen.PlanetName(rand)
 *
 * Parameters:
 *
 *   rand - optional, the <Rand> object to use to generate the name. if
 *          omitted, <Engine.rand> will be used
 *
 * Return:
 *
 *   name - a string containing the name
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
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
	lua_State *l = Pi::luaManager->GetLuaState();

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
