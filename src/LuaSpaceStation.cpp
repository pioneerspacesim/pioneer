#include "LuaObject.h"
#include "LuaUtils.h"
#include "SpaceStation.h"

static int l_spacestation_get_label(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua(l);
	lua_pushstring(l, s->GetLabel().c_str());
	return 1;
} 

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

static int l_spacestation_get_equipment_price(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua(l);
	int equip_type = luaL_checkinteger(l, 1);

	Sint64 cost = s->GetPrice(static_cast<Equip::Type>(equip_type));
	lua_pushnumber(l, cost * 0.01);
	return 1;
}

template <> const char *LuaSubObject<SpaceStation>::s_type = "SpaceStation";

template <> const luaL_reg LuaSubObject<SpaceStation>::s_methods[] = {
	{ "get_label", l_spacestation_get_label },

	{ "add_advert",    l_spacestation_add_advert    },
	{ "remove_advert", l_spacestation_remove_advert },

	{ "get_equipment_price", l_spacestation_get_equipment_price },
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<SpaceStation>::s_meta[] = {
	{ 0, 0 }
};
