// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "utils.h"
#include "ShipType.h"
#include "ShipFlavour.h"
#include "Pi.h"
#include "Serializer.h"

ShipFlavour::ShipFlavour()
{
	price = 0;
}

ShipFlavour::ShipFlavour(ShipType::Id id_)
{
	id = id_;
	regid = "XX-1111";
	regid[0] = 'A' + Pi::rng.Int32(26);
	regid[1] = 'A' + Pi::rng.Int32(26);
	int code = Pi::rng.Int32(10000);
	regid[3] = '0' + ((code / 1000) % 10);
	regid[4] = '0' + ((code /  100) % 10);
	regid[5] = '0' + ((code /   10) % 10);
	regid[6] = '0' + ((code /    1) % 10);
	price = std::max(ShipType::types[id].baseprice, 1);
	price = price + Pi::rng.Int32(price)/64;
}

// Pick a random ship type, and randomize the flavour
void ShipFlavour::MakeTrulyRandom(ShipFlavour &v, bool atmospheric)
{
	// only allow ships that can fit an atmospheric shield
	if (atmospheric) {
		const std::vector<ShipType::Id> &ships = ShipType::playable_atmospheric_ships;
		v = ShipFlavour(ships[Pi::rng.Int32(ships.size())]);
	} else {
		const std::vector<ShipType::Id> &ships = ShipType::player_ships;
		v = ShipFlavour(ships[Pi::rng.Int32(ships.size())]);
	}
}

void ShipFlavour::ApplyTo(Model *m) const
{
	m->SetLabel(regid);
}

void ShipFlavour::Save(Serializer::Writer &wr)
{
	wr.String(id);
	wr.Int32(price);
	wr.String(regid);
}

void ShipFlavour::Load(Serializer::Reader &rd)
{
	id = rd.String();
	price = rd.Int32();
	regid = rd.String();
}

static inline void _get_string(lua_State *l, const char *key, std::string &output)
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	output = lua_tostring(l, -1);
	lua_pop(l, 1);
}

static inline void _get_number(lua_State *l, const char *key, float &output)
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	output = lua_tonumber(l, -1);
	lua_pop(l, 1);
}

static inline void _get_colour(lua_State *l, const char *key, float rgba[4])
{
	lua_pushstring(l, key);
	lua_gettable(l, -2);
	_get_number(l, "r", rgba[0]);
	_get_number(l, "g", rgba[1]);
	_get_number(l, "b", rgba[2]);
	_get_number(l, "a", rgba[3]);
	lua_pop(l, 1);
}

ShipFlavour ShipFlavour::FromLuaTable(lua_State *l, int idx) {
	const int table = lua_absindex(l, idx);
	assert(lua_istable(l, table));

	LUA_DEBUG_START(l);

	lua_pushvalue(l, table);

	ShipFlavour f;

	_get_string(l, "id", f.id);
	_get_string(l, "regId", f.regid);

	float money;
	_get_number(l, "price", money);
	f.price = money*100.0;

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return f;
}
