#include "InfoView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "Ship.h"
#include "ShipCpanel.h"
#include "LmrModel.h"
#include "Render.h"

class InfoViewPage: public Gui::Fixed {
public:
	InfoViewPage(): Gui::Fixed(800, 500) {}
	virtual void UpdateInfo() = 0;
};

class MissionPage: public InfoViewPage {
public:
	MissionPage() {
	};

	virtual void Show() {
		UpdateInfo();
		InfoViewPage::Show();
	}

	virtual ~MissionPage() {
	}

	virtual void UpdateInfo() {
		const float YSEP = Gui::Screen::GetFontHeight() * 1.5;
		DeleteAllChildren();

		Gui::Label *l = new Gui::Label("Missions:");
		Add(l, 20, 20);

		l = new Gui::Label("Type");
		Add(l, 20, 20+YSEP*2);
		
		l = new Gui::Label("Client");
		Add(l, 100, 20+YSEP*2);
		
		l = new Gui::Label("Location");
		Add(l, 260, 20+YSEP*2);
		
		l = new Gui::Label("Due");
		Add(l, 420, 20+YSEP*2);
		
		l = new Gui::Label("Reward");
		Add(l, 580, 20+YSEP*2);

		l = new Gui::Label("Status");
		Add(l, 680, 20+YSEP*2);

		ShowChildren();

		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(760, 500);
		scroll->SetAdjustment(&portal->vscrollAdjust);

		const std::list<const Mission*> &missions = Pi::player->missions.GetAll();
		Gui::Fixed *innerbox = new Gui::Fixed(760, YSEP*3 * missions.size());

		float ypos = 0;
		for (std::list<const Mission*>::const_iterator i = missions.begin(); i != missions.end(); ++i) {
			SystemPath path = (*i)->location;
			StarSystem *s = StarSystem::GetCached(path);
			SBody *sbody = s->GetBodyByPath(&path);

			l = new Gui::Label((*i)->type);
			innerbox->Add(l, 0, ypos);
			
			l = new Gui::Label((*i)->client);
			innerbox->Add(l, 80, ypos);
			
			l = new Gui::Label(stringf(256, "%s,\n%s (%d, %d)", sbody->name.c_str(), s->GetName().c_str(), path.sectorX, path.sectorY));
			innerbox->Add(l, 240, ypos);
			
			l = new Gui::Label(format_date((*i)->due));
			innerbox->Add(l, 400, ypos);

			l = new Gui::Label(format_money((*i)->reward));
			innerbox->Add(l, 560, ypos);

			switch ((*i)->status) {
				case Mission::FAILED: l = new Gui::Label("#f00Failed"); break;
				case Mission::COMPLETED: l = new Gui::Label("#ff0Completed"); break;
				default:
				case Mission::ACTIVE: l = new Gui::Label("#0f0Active"); break;
			}
			innerbox->Add(l, 660, ypos);

			ypos += YSEP*3;
		}
		Add(portal, 20, 20 + YSEP*3);
		Add(scroll, 780, 20 + YSEP*3);
		scroll->ShowAll();
		portal->Add(innerbox);
		portal->ShowAll();
	}
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
			Pi::cpan->MsgLog()->Message("", std::string("Jettisonned 1 tonne of ")+EquipType::types[t].name);
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
			if (!(crime & (Uint64(1)<<i))) continue;
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
		std::string col1, col2;
		const ShipType &stype = Pi::player->GetShipType();
		col1 = "SHIP INFORMATION:  "+std::string(stype.name);
		col1 += "\n\nHyperdrive:"
			"\n\nCapacity:"
			"\nFree:"
			"\nUsed:"
			"\nAll-up weight:"
			"\n\nFront weapon:"
			"\nRear weapon:"
			"\n\nHyperspace range:\n\n";
		
		col2 = "\n\n";

		Equip::Type e = Pi::player->m_equipment.Get(Equip::SLOT_ENGINE);
		col2 += std::string(EquipType::types[e].name);

		const shipstats_t *stats;
		stats = Pi::player->CalcStats();
		snprintf(buf, sizeof(buf), "\n\n%dt\n"
					       "%dt\n"
					       "%dt\n"
					       "%dt", stats->max_capacity,
				stats->free_capacity, stats->used_capacity, stats->total_mass);
		col2 += std::string(buf);

		int numLasers = Pi::player->m_equipment.GetSlotSize(Equip::SLOT_LASER);
		if (numLasers >= 1) {
			e = Pi::player->m_equipment.Get(Equip::SLOT_LASER, 0);
			col2 += std::string("\n\n")+EquipType::types[e].name;
		} else {
			col2 += "\n\nno mounting";
		}
		if (numLasers >= 2) {
			e = Pi::player->m_equipment.Get(Equip::SLOT_LASER, 1);
			col2 += std::string("\n")+EquipType::types[e].name;
		} else {
			col2 += "\nno mounting";
		}

		snprintf(buf, sizeof(buf), "\n\n%.1f light years (%.1f max)", stats->hyperspace_range, stats->hyperspace_range_max);
		col2 += std::string(buf);

		for (int i=Equip::FIRST_SHIPEQUIP; i<=Equip::LAST_SHIPEQUIP; i++) {
			Equip::Type t = Equip::Type(i) ;
			Equip::Slot s = EquipType::types[t].slot;
			if ((s == Equip::SLOT_MISSILE) || (s == Equip::SLOT_ENGINE) || (s == Equip::SLOT_LASER)) continue;
			int num = Pi::player->m_equipment.Count(s, t);
			if (num == 1) {
				col1 += stringf(128, "%s\n", EquipType::types[t].name);
			} else if (num > 1) {
				col1 += stringf(128, "%d %ss\n", num, EquipType::types[t].name);
			}
		}

		info1->SetText(col1);
		info2->SetText(col2);
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
	m_doUpdate = true;
}

void InfoView::UpdateInfo()
{
	m_doUpdate = true;
}

void InfoView::Draw3D()
{
	/* XXX duplicated code in SpaceStationView.cpp */
	LmrObjParams params = {
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{},
		{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },

		{	// pColor[3]
		{ { 1.0f, 0.0f, 1.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.6f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
	};

	Pi::player->GetFlavour()->ApplyTo(&params);

	float guiscale[2];
	Gui::Screen::GetCoords2Pixels(guiscale);
	static float rot1, rot2;
	if (Pi::MouseButtonState(3)) {
		int m[2];
		Pi::GetMouseMotion(m);
		rot1 += -0.002*m[1];
		rot2 += -0.002*m[0];
	}
	else
	{
		rot1 += .5*Pi::GetFrameTime();
		rot2 += Pi::GetFrameTime();
	}
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
	
	Render::State::SetZnearZfar(1.0f, 10000.0f);
	
	float lightCol[] = { .5,.5,.5,0 };
	float lightDir[] = { 1,1,0,0 };

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightCol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);
	glEnable(GL_LIGHT0);
	glViewport(bx/guiscale[0], (Gui::Screen::GetHeight() - by - 320)/guiscale[1],
			320/guiscale[0], 320/guiscale[1]);
	
	matrix4x4f rot = matrix4x4f::RotateXMatrix(rot1);
	rot.RotateY(rot2);
	LmrModel *lmr_model = Pi::player->GetLmrModel();
	rot[14] = -1.5f * lmr_model->GetDrawClipRadius();

	lmr_model->Render(rot, &params);
	Render::State::UseProgram(0);
	Render::UnbindAllBuffers();
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
