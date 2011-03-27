#include "LuaObject.h"
#include "LuaUtils.h"
#include "SpaceStation.h"

static int l_spacestation_add_advert(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua();
	std::string title = LuaString::PullFromLua();

	if (!lua_isfunction(l, 1))
		luaL_typerror(l, 3, lua_typename(l, LUA_TFUNCTION));

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiAdverts");
		lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	}

	int ref = lua_objlen(l, -1);
	lua_pushinteger(l, ref);

	lua_newtable(l);

	lua_pushstring(l, "onActivate");
	lua_pushvalue(l, 1);
	lua_settable(l, -3);

	if (!lua_isnil(l, 2) && !lua_isfunction(l, 2))
		luaL_typerror(l, 4, lua_typename(l, LUA_TFUNCTION));
	else {
		lua_pushstring(l, "onDelete");
		lua_pushvalue(l, 2);
		lua_settable(l, -3);
	}

	lua_settable(l, -3);
	lua_pop(l, 1);

	s->BBAddAdvert(BBAdvert("LuaAdvert", ref, title));

	lua_pushinteger(l, ref);
	return 1;
} 

static int l_spacestation_remove_advert(lua_State *l)
{
	SpaceStation *s = LuaSpaceStation::PullFromLua();
	int ref = LuaInt::PullFromLua();

	lua_getfield(l, LUA_REGISTRYINDEX, "PiAdverts");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		return 0;
	}
	
	lua_pushinteger(l, ref);
	lua_gettable(l, -2);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 2);
		return 0;
	}

	s->BBRemoveAdvert("LuaAdvert", ref);

	lua_getfield(l, -1, "onDelete");
	if (lua_isnil(l, -1))
		lua_pop(l, 1);
	else {
		lua_pushinteger(l, ref);
		lua_call(l, 1, 0);
	}

	lua_pop(l, 1);

	lua_pushinteger(l, ref);
	lua_pushnil(l);
	lua_settable(l, -3);

	lua_pop(l, 0);

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

template <> const char *LuaObject<SpaceStation>::s_type = "SpaceStation";
template <> const char *LuaObject<SpaceStation>::s_inherit = "Body";

template <> const luaL_reg LuaObject<SpaceStation>::s_methods[] = {
	{ "AddAdvert",    l_spacestation_add_advert    },
	{ "RemoveAdvert", l_spacestation_remove_advert },

	{ "GetEquipmentPrice", l_spacestation_get_equipment_price },
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<SpaceStation>::s_meta[] = {
	{ 0, 0 }
};
