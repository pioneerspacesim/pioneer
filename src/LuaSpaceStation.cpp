#include "LuaObject.h"
#include "LuaUtils.h"
#include "SpaceStation.h"

static int l_spacestation_add_advert(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua();
	const char *module = LuaString::PullFromLua();
	int ref = LuaInt::PullFromLua();
	const char *desc = LuaString::PullFromLua();

	s->BBAddAdvert(BBAdvert(module, ref, desc));
	return 0;
} 

static int l_spacestation_remove_advert(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua();
	const char *module = LuaString::PullFromLua();
	int ref = LuaInt::PullFromLua();

	s->BBRemoveAdvert(module, ref);
	return 0;
} 

static int l_spacestation_get_equipment_price(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua();
	int equip_type = LuaInt::PullFromLua();

	Sint64 cost = s->GetPrice(static_cast<Equip::Type>(equip_type));
	lua_pushnumber(l, cost * 0.01);
	return 1;
}

template <> const char *LuaSubObject<SpaceStation>::s_type = "SpaceStation";
template <> const char *LuaSubObject<SpaceStation>::s_inherit = "Body";

template <> const luaL_reg LuaSubObject<SpaceStation>::s_methods[] = {
	{ "add_advert",    l_spacestation_add_advert    },
	{ "remove_advert", l_spacestation_remove_advert },

	{ "get_equipment_price", l_spacestation_get_equipment_price },
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<SpaceStation>::s_meta[] = {
	{ 0, 0 }
};
