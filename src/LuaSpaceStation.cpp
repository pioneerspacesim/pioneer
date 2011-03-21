#include "LuaObject.h"
#include "LuaUtils.h"
#include "SpaceStation.h"

static int l_spacestation_add_advert(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua(l);
	const char *module = luaL_checkstring(l, 1);
	int ref = luaL_checkinteger(l, 1);
	const char *desc = luaL_checkstring(l, 1);

	s->BBAddAdvert(BBAdvert(module, ref, desc));
	return 0;
} 

static int l_spacestation_remove_advert(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua(l);
	const char *module = luaL_checkstring(l, 1);
	int ref = luaL_checkinteger(l, 1);

	s->BBRemoveAdvert(module, ref);
	return 0;
} 

template <> const char *LuaSubObject<SpaceStation>::s_type = "SpaceStation";

template <> const luaL_reg LuaSubObject<SpaceStation>::s_methods[] = {
	{ "add_advert",    l_spacestation_add_advert    },
	{ "remove_advert", l_spacestation_remove_advert },
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<SpaceStation>::s_meta[] = {
	{ 0, 0 }
};
