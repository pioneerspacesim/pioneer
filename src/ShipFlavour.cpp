#include "libs.h"
#include "utils.h"
#include "ShipType.h"
#include "ShipFlavour.h"
#include "Pi.h"
#include "Serializer.h"
#include "LmrModel.h"

static const LmrMaterial s_white = { { 1.0f, 1.0f, 1.0f, 1.0f } };

ShipFlavour::ShipFlavour()
{
	regid[0] = 0;
	price = 0;
}

void ShipFlavour::MakeRandomColor(LmrMaterial &m)
{
	memset(&m, 0, sizeof(LmrMaterial));
	float r = Pi::rng.Double();
	float g = Pi::rng.Double();
	float b = Pi::rng.Double();

	float invmax = 1.0f / MAX(r, MAX(g, b));

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
	m.shininess = 50.0f + (float)Pi::rng.Double()*50.0f;
}

ShipFlavour::ShipFlavour(ShipType::Type type)
{
	this->type = type;
	snprintf(regid, sizeof(regid), "%c%c-%04d",
		'A' + Pi::rng.Int32(26),
		'A' + Pi::rng.Int32(26),
		Pi::rng.Int32(10000));
	price = ShipType::types[type].baseprice;
	price = price + Pi::rng.Int32(price)/64;

	MakeRandomColor(primaryColor);
	MakeRandomColor(secondaryColor);
}

void ShipFlavour::MakeTrulyRandom(ShipFlavour &v)
{
	v = ShipFlavour(ShipType::GetRandomType());
}

void ShipFlavour::ApplyTo(LmrObjParams *p) const
{
	p->argStrings[0] = regid;
	p->pMat[0] = primaryColor;
	p->pMat[1] = secondaryColor;
	p->pMat[2] = s_white;
}

void ShipFlavour::SaveLmrMaterial(LmrMaterial *m)
{
	using namespace Serializer::Write;
	for (int i=0; i<4; i++) wr_float(m->diffuse[i]);
	for (int i=0; i<4; i++) wr_float(m->specular[i]);
	for (int i=0; i<4; i++) wr_float(m->emissive[i]);
	wr_float(m->shininess);
}

void ShipFlavour::LoadLmrMaterial(LmrMaterial *m)
{
	using namespace Serializer::Read;
	if (IsOlderThan(13)) {
		MakeRandomColor(*m);
	} else {
		for (int i=0; i<4; i++) m->diffuse[i] = rd_float();
		for (int i=0; i<4; i++) m->specular[i] = rd_float();
		for (int i=0; i<4; i++) m->emissive[i] = rd_float();
		m->shininess = rd_float();
	}
}

void ShipFlavour::Save()
{
	using namespace Serializer::Write;
	wr_string(type);
	wr_int(price);
	wr_string(regid);
	SaveLmrMaterial(&primaryColor);
	SaveLmrMaterial(&secondaryColor);
}

void ShipFlavour::Load()
{
	using namespace Serializer::Read;
	type = rd_string();
	price = rd_int();
	rd_cstring2(regid, sizeof(regid));
	LoadLmrMaterial(&primaryColor);
	LoadLmrMaterial(&secondaryColor);
}

