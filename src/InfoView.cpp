#include "InfoView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "Ship.h"

InfoView::InfoView(): View()
{
	SetTransparency(true);

	info1 = new Gui::Label("");
	info2 = new Gui::Label("");
	Add(info1, 40, 40);
	Add(info2, 300, 40);
}

void InfoView::UpdateInfo()
{
	char buf[512];
	std::string nfo;
	const ShipType &stype = Pi::player->GetShipType();
	nfo = "SHIP INFORMATION:  "+std::string(stype.name);
	nfo += "\n\nDrive system:"
		"\n\nCapacity:"
		"\nFree:"
		"\nUsed:"
		"\nAll-up weight:"
		"\n\nFront weapon:"
		"\nRear weapon:"
		"\n\nHyperspace range:";
	info1->SetText(nfo);
	
	nfo = "\n\n";

	Equip::Type e = Pi::player->m_equipment.Get(Equip::SLOT_ENGINE);
	nfo += std::string(EquipType::types[e].name);

	const shipstats_t *stats;
	stats = Pi::player->CalcStats();
	snprintf(buf, sizeof(buf), "\n\n%dt\n"
				       "%dt\n"
			               "%dt\n"
				       "%dt", stats->max_capacity,
			stats->free_capacity, stats->used_capacity, stats->total_mass);
	nfo += std::string(buf);

	e = Pi::player->m_equipment.Get(Equip::SLOT_LASER, 0);
	nfo += std::string("\n\n")+EquipType::types[e].name;
	e = Pi::player->m_equipment.Get(Equip::SLOT_LASER, 1);
	nfo += std::string("\n")+EquipType::types[e].name;

	snprintf(buf, sizeof(buf), "\n\n%.1f light years", stats->hyperspace_range);
	nfo += std::string(buf);

	info2->SetText(nfo);
}

static ObjParams params = {
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { 1.0f, 0.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

	// pText[3][256]	
	{ "IR-L33T", "ME TOO" },
};

void InfoView::Draw3D()
{
	static float rot1, rot2;
	rot1 += .5*Pi::GetFrameTime();
	rot2 += Pi::GetFrameTime();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// why the hell do i give these functions such big names..
	glFrustum(-Pi::GetScrWidth()*.5, Pi::GetScrWidth()*.5,
		  -Pi::GetScrHeight()*.5, Pi::GetScrHeight()*.5,
		   Pi::GetScrWidth()*.5, 100000);
	glDepthRange (-10, -100000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,.2,.4,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float lightCol[] = { 1,1,1 };
	float lightDir[] = { 1,0,0 };

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreSetDirLight (lightCol, lightDir);
	sbreSetViewport(Pi::GetScrWidth(), Pi::GetScrHeight(), Pi::GetScrWidth()*0.5, 5.0f, 100000.0f, 0.0f, 1.0f);
	// sod you sbre i want my own viewport!
	glViewport(Pi::GetScrWidth()/4, 0, Pi::GetScrWidth(), Pi::GetScrHeight());
	
	Vector p; p.x = 0; p.y = 0; p.z = 100;
	matrix4x4d rot = matrix4x4d::RotateXMatrix(rot1);
	rot.RotateY(rot2);
	Matrix m;
	m.x1 = rot[0]; m.x2 = rot[4]; m.x3 = -rot[8];
	m.y1 = rot[1]; m.y2 = rot[5]; m.y3 = -rot[9];
	m.z1 = -rot[2]; m.z2 = -rot[6]; m.z3 = rot[10];
	const ShipType &stype = Pi::player->GetShipType();

	sbreSetDepthRange (Pi::GetScrWidth()*0.5f, 0.0f, 1.0f);
	sbreRenderModel(&p, &m, stype.sbreModel, &params);
	glPopAttrib();
}

void InfoView::Update()
{
}
