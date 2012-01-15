#include "LuaNameGen.h"
#include "LuaObject.h"
#include "LuaRand.h"
#include "LuaSBody.h"
#include "mtrand.h"

static const std::string defaultMaleFullName("Tom Morton");
static const std::string defaultFemaleFullName("Thomasina Mortonella");
static const std::string defaultSurname("Jameson");
static const std::string defaultBodyName("Planet Rock");

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

	if (!GetNameGenFunc(l, "FullName"))
		return isFemale ? defaultFemaleFullName : defaultMaleFullName;

	lua_pushboolean(l, isFemale);
	LuaRand::PushToLua(&rng);
	pi_lua_protected_call(l, 2, 1);

	std::string fullname = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	return fullname;
}

std::string LuaNameGen::Surname(MTRand &rng)
{
	lua_State *l = m_luaManager->GetLuaState();

	if (!GetNameGenFunc(l, "Surname"))
		return defaultSurname;

	LuaRand::PushToLua(&rng);
	pi_lua_protected_call(l, 1, 1);

	std::string surname = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	return surname;
}

std::string LuaNameGen::BodyName(SBody *body, MTRand &rng)
{
	lua_State *l = m_luaManager->GetLuaState();

	if (!GetNameGenFunc(l, "BodyName"))
		return defaultBodyName;

	LuaSBody::PushToLua(body);
	LuaRand::PushToLua(&rng);
	pi_lua_protected_call(l, 2, 1);

	std::string bodyname = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	return bodyname;
}
