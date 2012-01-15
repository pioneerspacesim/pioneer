#include "LuaNameGen.h"
#include "LuaObject.h"
#include "LuaRand.h"
#include "LuaSBody.h"
#include "mtrand.h"

static const std::string defaultMaleFullName("Tom Morton");
static const std::string defaultFemaleFullName("Thomasina Mortonella");
static const std::string defaultSurname("Jameson");

static bool GetNameGenFunc(lua_State *l, const char *func)
{
	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_GLOBALSINDEX, "NameGen");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		LUA_DEBUG_END(l, 0);
		return false;
	}

	lua_getfield(l, -1, func);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 2);
		LUA_DEBUG_END(l, 0);
		return false;
	}

	lua_remove(l, -2);

	LUA_DEBUG_END(l, 1);
	return true;
}

std::string LuaNameGen::FullName(bool isFemale, MTRand &rng)
{
	lua_State *l = m_luaManager->GetLuaState();

	GetNameGenFunc(l, "FullName");
	LuaRand::PushToLua(&rng);
	lua_pushboolean(l, isFemale);
	pi_lua_protected_call(l, 2, 1);

	std::string fullname = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	return fullname;
}

std::string LuaNameGen::Surname(MTRand &rng)
{
	lua_State *l = m_luaManager->GetLuaState();

	GetNameGenFunc(l, "Surname");
	LuaRand::PushToLua(&rng);
	pi_lua_protected_call(l, 1, 1);

	std::string surname = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	return surname;
}

std::string LuaNameGen::BodyName(SBody *body, MTRand &rng)
{
	lua_State *l = m_luaManager->GetLuaState();

	GetNameGenFunc(l, "BodyName");
	LuaSBody::PushToLua(body);
	LuaRand::PushToLua(&rng);
	pi_lua_protected_call(l, 2, 1);

	std::string bodyname = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	return bodyname;
}

#if 0
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
#endif
