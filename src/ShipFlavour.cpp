#include "libs.h"
#include "utils.h"
#include "ShipType.h"
#include "ShipFlavour.h"
#include "Pi.h"
#include "Serializer.h"
#include "sbre/sbre.h"

ShipFlavour::ShipFlavour()
{
	memset(this, 0, sizeof(ShipFlavour));
}

void ShipFlavour::MakeRandomColor(Material &m)
{
	memset(&m, 0, sizeof(Material));
	float r = Pi::rng.Double();
	float g = Pi::rng.Double();
	float b = Pi::rng.Double();

	float invmax = 1.0f / MAX(r, MAX(g, b));

	r *= invmax;
	g *= invmax;
	b *= invmax;

	m.pDiff[0] = 0.5f * r;
	m.pDiff[1] = 0.5f * g;
	m.pDiff[2] = 0.5f * b;
	m.pSpec[0] = r;
	m.pSpec[1] = g;
	m.pSpec[2] = b;
	m.shiny = 50.0f + (float)Pi::rng.Double()*50.0f;
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
	v = ShipFlavour(static_cast<ShipType::Type>(Pi::rng.Int32(ShipType::END)));
}

void ShipFlavour::ApplyTo(ObjParams *p) const
{
	memset(p->pText[0], 0, sizeof(p->pText[0]));
	strncpy(p->pText[0], regid, sizeof(p->pText[0]));
	p->pColor[0] = primaryColor;
	p->pColor[1] = secondaryColor;
}

void ShipFlavour::Save()
{
	using namespace Serializer::Write;
	wr_int((int)type);
	wr_int(price);
	wr_string(regid);
}

void ShipFlavour::Load()
{
	using namespace Serializer::Read;
	type = static_cast<ShipType::Type>(rd_int());
	price = rd_int();
	rd_cstring2(regid, sizeof(regid));
}

