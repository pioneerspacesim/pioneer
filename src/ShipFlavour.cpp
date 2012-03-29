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

ShipFlavour::ShipFlavour(ShipType::Type type_)
{
	type = type_;
	regid = "XX-1111";
	regid[0] = 'A' + Pi::rng.Int32(26);
	regid[1] = 'A' + Pi::rng.Int32(26);
	int code = Pi::rng.Int32(10000);
	regid[3] = '0' + ((code / 1000) % 10);
	regid[4] = '0' + ((code /  100) % 10);
	regid[5] = '0' + ((code /   10) % 10);
	regid[6] = '0' + ((code /    1) % 10);
	price = std::max(ShipType::types[type].baseprice, 1);
	price = price + Pi::rng.Int32(price)/64;

	MakeRandomColor(primaryColor);
	MakeRandomColor(secondaryColor);
}

void ShipFlavour::MakeTrulyRandom(ShipFlavour &v)
{
	const std::vector<ShipType::Type> &ships = ShipType::player_ships;
	v = ShipFlavour(ships[Pi::rng.Int32(ships.size())]);
}

void ShipFlavour::ApplyTo(LmrObjParams *p) const
{
	p->label = regid.c_str();
	p->pMat[0] = primaryColor;
	p->pMat[1] = secondaryColor;
	p->pMat[2] = s_white;
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
	wr.String(type);
	wr.Int32(price);
	wr.String(regid);
	SaveLmrMaterial(wr, &primaryColor);
	SaveLmrMaterial(wr, &secondaryColor);
}

void ShipFlavour::Load(Serializer::Reader &rd)
{
	type = rd.String();
	price = rd.Int32();
	regid = rd.String();
	LoadLmrMaterial(rd, &primaryColor);
	LoadLmrMaterial(rd, &secondaryColor);
}

