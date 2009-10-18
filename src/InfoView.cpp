#include "InfoView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "Ship.h"
#include "ShipCpanel.h"
#include "Mission.h"

class InfoViewPage: public Gui::Fixed {
public:
	InfoViewPage(): Gui::Fixed(800, 500) {}
	virtual void UpdateInfo() = 0;
};

class MissionPage: public InfoViewPage {
public:
	MissionPage() {
		m_onMissionListChangedConnection = Pi::onPlayerMissionListChanged.connect(
				sigc::mem_fun(this, &MissionPage::UpdateInfo));
	};

	virtual ~MissionPage() {
		m_onMissionListChangedConnection.disconnect();
	}

	virtual void UpdateInfo() {
		const float YSEP = Gui::Screen::GetFontHeight() * 1.5;
		DeleteAllChildren();

		Gui::Label *l = new Gui::Label("Missions:");
		Add(l, 20, 20);
		l->Show();

		l = new Gui::Label("Status");
		Add(l, 20, 20+YSEP*2);
		l->Show();
		
		l = new Gui::Label("Due");
		Add(l, 100, 20+YSEP*2);
		l->Show();
		
		l = new Gui::Label("Client");
		Add(l, 160, 20+YSEP*2);
		l->Show();
		
		l = new Gui::Label("Description");
		Add(l, 300, 20+YSEP*2);
		l->Show();

		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(760, 500);
		scroll->SetAdjustment(&portal->vscrollAdjust);

		const std::list<Mission*> missions = Pi::player->GetMissions();
		Gui::Fixed *innerbox = new Gui::Fixed(760, YSEP*3 * missions.size());

		float ypos = 0;
		for (std::list<Mission*>::const_iterator i = missions.begin();
				i != missions.end(); ++i) {
			switch ((*i)->GetStatus()) {
				case Mission::FAILED: l = new Gui::Label("#f00Failed"); break;
				case Mission::COMPLETED: l = new Gui::Label("#ff0Completed"); break;
				default:
				case Mission::ACTIVE: l = new Gui::Label("#0f0Active"); break;
			}
			innerbox->Add(l, 0, ypos);
			l->Show();
			
			l = new Gui::Label(format_money((*i)->GetPayoff()));
			innerbox->Add(l, 80, ypos);
			l->Show();
			
			l = new Gui::Label((*i)->GetClientName());
			innerbox->Add(l, 140, ypos);
			l->Show();

			l = new Gui::Label((*i)->GetMissionText());
			innerbox->Add(l, 280, ypos);
			l->Show();

			ypos += YSEP*3;
		}
		Add(portal, 20, 20 + YSEP*3);
		Add(scroll, 780, 20 + YSEP*3);
		scroll->ShowAll();
		portal->Add(innerbox);
		portal->ShowAll();
	}
private:
	void JettisonCargo(Equip::Type t) {
		if (Pi::player->Jettison(t)) {
			Pi::cpan->SetTemporaryMessage(0, std::string("Jettisonned 1 tonne of ")+EquipType::types[t].name);
			Pi::infoView->UpdateInfo();
		}
	}
	sigc::connection m_onMissionListChangedConnection;
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

class PersonalPage: public InfoViewPage {
public:
	PersonalPage() {
	};

	virtual void UpdateInfo() {
		Sint64 crime, fine;
		Polit::GetCrime(&crime, &fine);
		const float YSEP = Gui::Screen::GetFontHeight() * 1.5;
		DeleteAllChildren();

		float ypos = 40.0f;
		Add((new Gui::Label("COMBAT RATING:"))->Shadow(true), 40, ypos);
		Add(new Gui::Label(Pi::combatRating[ Pi::CombatRating(Pi::player->GetKillCount()) ]), 40, ypos+YSEP);

		ypos = 160.0f;
		Add((new Gui::Label("CRIMINAL RECORD:"))->Shadow(true), 40, ypos);
		for (int i=0; i<64; i++) {
			if (!(crime & (1<<i))) continue;
			if (!Polit::crimeNames[i]) continue;
			ypos += YSEP;
			Add(new Gui::Label(Polit::crimeNames[i]), 40, ypos);
		}
		ShowChildren();
	}
};

class ShipInfoPage: public InfoViewPage {
public:
	ShipInfoPage() {
		info1 = new Gui::Label("");
		info2 = new Gui::Label("");
		Add(info1, 40, 40);
		Add(info2, 250, 40);
		ShowAll();
	};

	virtual void UpdateInfo() {
		char buf[512];
		std::string nfo;
		const ShipType &stype = Pi::player->GetShipType();
		nfo = "SHIP INFORMATION:  "+std::string(stype.name);
		nfo += "\n\nHyperdrive:"
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

		int numLasers = Pi::player->m_equipment.GetSlotSize(Equip::SLOT_LASER);
		if (numLasers >= 1) {
			e = Pi::player->m_equipment.Get(Equip::SLOT_LASER, 0);
			nfo += std::string("\n\n")+EquipType::types[e].name;
		} else {
			nfo += "\n\nno mounting";
		}
		if (numLasers >= 2) {
			e = Pi::player->m_equipment.Get(Equip::SLOT_LASER, 1);
			nfo += std::string("\n")+EquipType::types[e].name;
		} else {
			nfo += "\nno mounting";
		}

		snprintf(buf, sizeof(buf), "\n\n%.1f light years (%.1f max)", stats->hyperspace_range, stats->hyperspace_range_max);
		nfo += std::string(buf);

		info2->SetText(nfo);
		this->ResizeRequest();
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

	page = new PersonalPage();
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label("Reputation"), page);
	
	page = new CargoPage();
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label("Cargo"), page);
	
	page = new MissionPage();
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label("Missions"), page);
	
	Add(m_tabs, 0, 0);
//	m_tabs->SetShortcut(SDLK_F3, KMOD_NONE);
	m_doUpdate = true;
}

void InfoView::UpdateInfo()
{
	m_doUpdate = true;
}

void InfoView::Draw3D()
{
	/* XXX duplicated code in SpaceStationView.cpp */
	ObjParams params = {
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },

		{	// pColor[3]
		{ { 1.0f, 0.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
	};

	Pi::player->GetFlavour()->ApplyTo(&params);

	float guiscale[2];
	Gui::Screen::GetCoords2Pixels(guiscale);
	static float rot1, rot2;
	rot1 += .5*Pi::GetFrameTime();
	rot2 += Pi::GetFrameTime();
	glClearColor(0,.2,.4,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if (m_tabs->GetCurrentPage() != 0) return;
	
	const float bx = 450;
	const float by = 50;
	Gui::Screen::EnterOrtho();
	glColor3f(0,0,0);
	glBegin(GL_QUADS); {
		glVertex2f(bx,by);
		glVertex2f(bx,by+320);
		glVertex2f(bx+320,by+320);
		glVertex2f(bx+320,by);
	} glEnd();
	Gui::Screen::LeaveOrtho();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-.5, .5, -.5, .5, 1.0f, 10000.0f);
	glDepthRange (0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	float lightCol[] = { 1,1,1 };
	float lightDir[] = { 1,1,0 };

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreSetDirLight (lightCol, lightDir);
	glViewport(bx/guiscale[0], (Gui::Screen::GetHeight() - by - 320)/guiscale[1],
			320/guiscale[0], 320/guiscale[1]);
	
	matrix4x4d rot = matrix4x4d::RotateXMatrix(rot1);
	rot.RotateY(rot2);
	int sbre_model = Pi::player->GetSbreModel();

	vector3d p(0, 0, -2.1 * sbreGetModelRadius(sbre_model));
	sbreSetDepthRange (Pi::GetScrWidth()*0.5f, 0.0f, 1.0f);
	sbreRenderModel(&p.x, &rot[0], sbre_model, &params);
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
