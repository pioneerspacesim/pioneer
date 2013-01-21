// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "utils.h"
#include "ShipType.h"
#include "ShipFlavour.h"
#include "Pi.h"
#include "Serializer.h"
#include "LmrModel.h"

static const LmrMaterial s_white = {
    { 1.0f, 1.0f, 1.0f, 1.0f }, //diffuse
    { 1.0f, 1.0f, 1.0f, 1.0f }, //specular
    { 1.0f, 1.0f, 1.0f, 1.0f }, //emissive
    0.0f //shinyness
};

ShipFlavour::ShipFlavour()
{
	price = 0;
}

void ShipFlavour::MakeRandomColor(LmrMaterial &m)
{
	memset(&m, 0, sizeof(LmrMaterial));
	float r = Pi::rng.Double();
	float g = Pi::rng.Double();
	float b = Pi::rng.Double();

	float invmax = 1.0f / std::max(r, std::max(g, b));

	r *= invmax;
	g *= invmax;
	b *= invmax;

	m.diffuse[0] = 0.5f * r;
	m.diffuse[1] = 0.5f * g;
	m.diffuse[2] = 0.5f * b;
	m.diffuse[3] = 1.0f;
	m.specular[0] = r;
	m.specular[1] = g;
	m.specular[2] = b;
	m.shininess = 50.0f + float(Pi::rng.Double())*50.0f;
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

	MakeRandomColor(primaryColor);
	MakeRandomColor(secondaryColor);
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

void ShipFlavour::ApplyTo(LmrObjParams *p) const
{
	p->pMat[0] = primaryColor;
	p->pMat[1] = secondaryColor;
	p->pMat[2] = s_white;
}

void ShipFlavour::ApplyTo(ModelBase *m) const
{
	m->SetLabel(regid);
}

void ShipFlavour::SaveLmrMaterial(Serializer::Writer &wr, LmrMaterial *m)
{
	for (int i=0; i<4; i++) wr.Float(m->diffuse[i]);
	for (int i=0; i<4; i++) wr.Float(m->specular[i]);
	for (int i=0; i<4; i++) wr.Float(m->emissive[i]);
	wr.Float(m->shininess);
}

void ShipFlavour::LoadLmrMaterial(Serializer::Reader &rd, LmrMaterial *m)
{
	for (int i=0; i<4; i++) m->diffuse[i] = rd.Float();
	for (int i=0; i<4; i++) m->specular[i] = rd.Float();
	for (int i=0; i<4; i++) m->emissive[i] = rd.Float();
	m->shininess = rd.Float();
}

void ShipFlavour::Save(Serializer::Writer &wr)
{
	wr.String(id);
	wr.Int32(price);
	wr.String(regid);
	SaveLmrMaterial(wr, &primaryColor);
	SaveLmrMaterial(wr, &secondaryColor);
}

void ShipFlavour::Load(Serializer::Reader &rd)
{
	id = rd.String();
	price = rd.Int32();
	regid = rd.String();
	LoadLmrMaterial(rd, &primaryColor);
	LoadLmrMaterial(rd, &secondaryColor);
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

	lua_getfield(l, -1, "primaryColour");
	_get_colour(l, "diffuse", f.primaryColor.diffuse);
	_get_colour(l, "specular", f.primaryColor.specular);
	_get_colour(l, "emissive", f.primaryColor.emissive);
	_get_number(l, "shininess", f.primaryColor.shininess);
	lua_pop(l, 1);

	lua_getfield(l, -1, "secondaryColour");
	_get_colour(l, "diffuse", f.secondaryColor.diffuse);
	_get_colour(l, "specular", f.secondaryColor.specular);
	_get_colour(l, "emissive", f.secondaryColor.emissive);
	_get_number(l, "shininess", f.secondaryColor.shininess);
	lua_pop(l, 1);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return f;
}
