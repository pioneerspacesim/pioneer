#include "InfoView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "Ship.h"
#include "ShipCpanel.h"

class InfoViewPage: public Gui::Fixed {
public:
	virtual void UpdateInfo() = 0;
};

class CargoPage: public InfoViewPage {
public:
	CargoPage() {
	};

	virtual void UpdateInfo() {
		const float YSEP = Gui::Screen::GetFontHeight() * 1.5;
		DeleteAllChildren();
		Add(new Gui::Label("Cargo Inventory:"), 40, 40);
		Add(new Gui::Label("Jettison"), 40, 40+YSEP*2);
		float ypos = 40 + 3*YSEP;
		for (int i=1; i<Equip::TYPE_MAX; i++) {
			if (EquipType::types[i].slot != Equip::SLOT_CARGO) continue;
			const int gotNum = Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(i));
			if (!gotNum) continue;
			Gui::Button *b = new Gui::SolidButton();
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &CargoPage::JettisonCargo), static_cast<Equip::Type>(i)));
			Add(b, 40, ypos);
			Add(new Gui::Label(EquipType::types[i].name), 70, ypos);
			char buf[128];
			snprintf(buf, sizeof(buf), "%dt", gotNum*EquipType::types[i].mass);
			Add(new Gui::Label(buf), 300, ypos);
			ypos += YSEP;
		}
		ShowChildren();
	}
private:
	void JettisonCargo(Equip::Type t) {
		if (Pi::player->Jettison(t)) {
			Pi::cpan->SetTemporaryMessage(0, std::string("Jettisonned 1 tonne of ")+EquipType::types[t].name);
			Pi::infoView->UpdateInfo();
		}
	}
};

class ShipInfoPage: public InfoViewPage {
public:
	ShipInfoPage() {
		info1 = new Gui::Label("");
		info2 = new Gui::Label("");
		Add(info1, 40, 40);
		Add(info2, 300, 40);
		ShowAll();
	};

	virtual void UpdateInfo() {
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
private:
	Gui::Label *info1, *info2;
};

InfoView::InfoView(): View()
{
	SetTransparency(true);

	m_tabs = new Gui::Tabbed();

	InfoViewPage *page = new ShipInfoPage();
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label("Ship Information"), page);
	page = new CargoPage();
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label("Cargo"), page);
	Add(m_tabs, 0, 0);
//	m_tabs->SetShortcut(SDLK_F3, KMOD_NONE);
	m_doUpdate = true;
}

void InfoView::UpdateInfo()
{
	m_doUpdate = true;
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
	glClearColor(0,.2,.4,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	Gui::Screen::EnterOrtho();
	glColor3f(0,0,0);
	glBegin(GL_QUADS); {
		const float bx = 450;
		const float by = 50;
		glVertex2f(bx,by);
		glVertex2f(bx,by+300);
		glVertex2f(bx+320,by+300);
		glVertex2f(bx+320,by);
	} glEnd();
	Gui::Screen::LeaveOrtho();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float fracH = 1.0 / Pi::GetScrAspect();
	glFrustum(-1, 1, -fracH, fracH, 1.0f, 10000.0f);
	glDepthRange (-10, -100000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	float lightCol[] = { 1,1,1 };
	float lightDir[] = { 1,1,0 };

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreSetDirLight (lightCol, lightDir);
	glViewport(Pi::GetScrWidth()/3.8, Pi::GetScrHeight()/6, Pi::GetScrWidth(), Pi::GetScrHeight());
	
	Vector p; p.x = 0; p.y = 0; p.z = -100;
	matrix4x4d rot = matrix4x4d::RotateXMatrix(rot1);
	rot.RotateY(rot2);
	Matrix m;
	m.x1 = rot[0]; m.x2 = rot[4]; m.x3 = rot[8];
	m.y1 = rot[1]; m.y2 = rot[5]; m.y3 = rot[9];
	m.z1 = rot[2]; m.z2 = rot[6]; m.z3 = rot[10];
	const ShipType &stype = Pi::player->GetShipType();

	sbreSetDepthRange (Pi::GetScrWidth()*0.5f, 0.0f, 1.0f);
	sbreRenderModel(&p, &m, stype.sbreModel, &params);
	glPopAttrib();
}

void InfoView::Update()
{
	if (m_doUpdate) {
		for (std::list<InfoViewPage*>::iterator i = m_pages.begin(); i!=m_pages.end(); ++i) {
			(*i)->UpdateInfo();
		}
		m_doUpdate = false;
	}
}

void InfoView::NextPage()
{
	m_tabs->OnActivate();
}
