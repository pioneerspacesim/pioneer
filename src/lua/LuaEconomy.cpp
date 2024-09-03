// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "galaxy/Economy.h"

#include "LuaEconomy.h"

static void pi_lua_generic_push(lua_State *l, GalacticEconomy::CommodityInfo info)
{
	lua_newtable(l);

	pi_lua_settable(l, "id", int(info.id));
	pi_lua_settable(l, "producer", int(info.producer));
	pi_lua_settable(l, "name", info.name);
	pi_lua_settable(l, "l10n_key", info.l10n_key);

	lua_newtable(l);
	size_t count = 1;
	for (auto &input : info.inputs) {
		lua_pushinteger(l, count++);
		lua_newtable(l);
		pi_lua_settable(l, "id", int(input.first));
		pi_lua_settable(l, "amount", input.second.ToDouble());
		lua_settable(l, -3);
	}
	lua_setfield(l, -2, "inputs");

	pi_lua_settable(l, "price", info.price);
}

static int l_economy_get_commodities(lua_State *l)
{
	LuaTable commodityTable(l);

	for (const auto &commodity : GalacticEconomy::Commodities()) {
		commodityTable.Set(commodity.name, commodity);
	}

	return 1;
}

static void pi_lua_generic_push(lua_State *l, GalacticEconomy::EconomyInfo info)
{
	lua_newtable(l);

	pi_lua_settable(l, "id", int(info.id));
	pi_lua_settable(l, "name", info.name);

	lua_newtable(l);
	pi_lua_settable(l, "small", info.l10n_key.small);
	pi_lua_settable(l, "medium", info.l10n_key.medium);
	pi_lua_settable(l, "large", info.l10n_key.large);
	pi_lua_settable(l, "huge", info.l10n_key.huge);
	lua_setfield(l, -2, "l10n_key");

	lua_newtable(l);
	pi_lua_settable(l, "agricultural", info.affinity.agricultural.ToDouble());
	pi_lua_settable(l, "industrial",   info.affinity.industrial.ToDouble());
	pi_lua_settable(l, "metallicity",  info.affinity.metallicity.ToDouble());
	lua_setfield(l, -2, "affinity");

	lua_newtable(l);
	pi_lua_settable(l, "agricultural", info.generation.agricultural.ToDouble());
	pi_lua_settable(l, "industrial", info.generation.industrial.ToDouble());
	pi_lua_settable(l, "metallicity", info.generation.metallicity.ToDouble());
	pi_lua_settable(l, "population", info.generation.population.ToDouble());
	pi_lua_settable(l, "random", info.generation.random.ToDouble());
	lua_setfield(l, -2, "generation");
}

static int l_economy_get_economies(lua_State *l)
{
	LuaTable economyTable(l);

	for (const auto &economy : GalacticEconomy::Economies()) {
		economyTable.Set(economy.name, economy);
	}

	return 1;
}

static int l_economy_get_commodity_by_id(lua_State *l)
{
	GalacticEconomy::CommodityId comm_id = luaL_checkinteger(l, 1);

	LuaPush(l, GalacticEconomy::GetCommodityById(comm_id));
	return 1;
}

static int l_economy_get_economy_by_id(lua_State *l)
{
	GalacticEconomy::EconomyId econ_id = luaL_checkinteger(l, 1);

	LuaPush(l, GalacticEconomy::GetEconomyById(econ_id));
	return 1;
}

void LuaEconomy::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	static LuaMetaTypeGeneric s_metaType("Economy");
	s_metaType.CreateMetaType(l);

	s_metaType.StartRecording()
		.AddFunction("GetCommodities", l_economy_get_commodities)
		.AddFunction("GetCommodityById", l_economy_get_commodity_by_id)
		.AddFunction("GetEconomies", l_economy_get_economies)
		.AddFunction("GetEconomyById", l_economy_get_economy_by_id)
		.StopRecording();

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(&s_metaType);
	lua_setfield(l, -2, "Economy");
}
