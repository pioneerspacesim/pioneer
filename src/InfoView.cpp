#include "InfoView.h"
#include "Pi.h"
#include "Player.h"
#include "WorldView.h"
#include "Ship.h"
#include "ShipCpanel.h"
#include "LmrModel.h"
#include "render/Render.h"
#include "Lang.h"
#include "StringF.h"

class InfoViewPage: public Gui::Fixed {
public:
	InfoViewPage(InfoView *v): Gui::Fixed(800, 500), m_infoView(v) {}
	virtual void UpdateInfo() = 0;

protected:
	InfoView *m_infoView;
};

class MissionPage: public InfoViewPage {
public:
	MissionPage(InfoView *v) : InfoViewPage(v) {};

	virtual void Show() {
		UpdateInfo();
		InfoViewPage::Show();
		m_infoView->HideSpinner();
	}

	virtual ~MissionPage() {
	}

	virtual void UpdateInfo() {
		const float YSEP = Gui::Screen::GetFontHeight() * 1.5f;
		DeleteAllChildren();

		Gui::Label *l = new Gui::Label(Lang::MISSIONS);
		Add(l, 20, 20);

		l = new Gui::Label(Lang::TYPE);
		Add(l, 20, 20+YSEP*2);
		
		l = new Gui::Label(Lang::CLIENT);
		Add(l, 100, 20+YSEP*2);
		
		l = new Gui::Label(Lang::LOCATION);
		Add(l, 260, 20+YSEP*2);
		
		l = new Gui::Label(Lang::DUE);
		Add(l, 420, 20+YSEP*2);
		
		l = new Gui::Label(Lang::REWARD);
		Add(l, 580, 20+YSEP*2);

		l = new Gui::Label(Lang::STATUS);
		Add(l, 680, 20+YSEP*2);

		ShowChildren();

		Gui::VScrollBar *scroll = new Gui::VScrollBar();
		Gui::VScrollPortal *portal = new Gui::VScrollPortal(760);
		scroll->SetAdjustment(&portal->vscrollAdjust);

		const std::list<const Mission*> &missions = Pi::player->missions.GetAll();
		Gui::Fixed *innerbox = new Gui::Fixed(760, YSEP*3 * missions.size());

		float ypos = 0;
		for (std::list<const Mission*>::const_iterator i = missions.begin(); i != missions.end(); ++i) {
			SystemPath path = (*i)->location;
			RefCountedPtr<StarSystem> s = StarSystem::GetCached(path);

			l = new Gui::Label((*i)->type);
			innerbox->Add(l, 0, ypos);
			
			l = new Gui::Label((*i)->client);
			innerbox->Add(l, 80, ypos);
			
			if (!path.IsBodyPath())
				l = new Gui::Label(stringf("%0 [%1{d},%2{d},%3{d}]", s->GetName().c_str(), path.sectorX, path.sectorY, path.sectorZ));
			else
				l = new Gui::Label(stringf("%0\n%1 [%2{d},%3{d},%4{d}]", s->GetBodyByPath(&path)->name.c_str(), s->GetName().c_str(), path.sectorX, path.sectorY, path.sectorZ));
			innerbox->Add(l, 240, ypos);
			
			l = new Gui::Label(format_date((*i)->due));
			innerbox->Add(l, 400, ypos);

			l = new Gui::Label(format_money((*i)->reward));
			innerbox->Add(l, 560, ypos);

			switch ((*i)->status) {
                case Mission::FAILED: l = new Gui::Label(std::string("#f00")+std::string(Lang::FAILED)); break;
                case Mission::COMPLETED: l = new Gui::Label(std::string("#ff0")+std::string(Lang::COMPLETED)); break;
				default:
                case Mission::ACTIVE: l = new Gui::Label(std::string("#0f0")+std::string(Lang::ACTIVE)); break;
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
	CargoPage(InfoView *v) : InfoViewPage(v) {};

	virtual void Show() {
		InfoViewPage::Show();
		m_infoView->ShowSpinner();
	}

	virtual void UpdateInfo() {
		const float YSEP = Gui::Screen::GetFontHeight() * 1.5f;
		DeleteAllChildren();
		Add(new Gui::Label(Lang::CARGO_INVENTORY), 40, 40);
		Add(new Gui::Label(Lang::JETTISON), 40, 40+YSEP*2);
		float ypos = 40 + 3*YSEP;
		for (int i=1; i<Equip::TYPE_MAX; i++) {
			if (Equip::types[i].slot != Equip::SLOT_CARGO) continue;
			const int gotNum = Pi::player->m_equipment.Count(Equip::SLOT_CARGO, static_cast<Equip::Type>(i));
			if (!gotNum) continue;
			Gui::Button *b = new Gui::SolidButton();
			b->onClick.connect(sigc::bind(sigc::mem_fun(this, &CargoPage::JettisonCargo), static_cast<Equip::Type>(i)));
			Add(b, 40, ypos);
			Add(new Gui::Label(Equip::types[i].name), 70, ypos);
			char buf[128];
			snprintf(buf, sizeof(buf), "%dt", gotNum*Equip::types[i].mass);
			Add(new Gui::Label(buf), 300, ypos);
			ypos += YSEP;
		}
		ShowChildren();
	}
private:
	void JettisonCargo(Equip::Type t) {
		if (Pi::player->Jettison(t)) {
			Pi::cpan->MsgLog()->Message("", stringf(Lang::JETTISONED_1T_OF_X, formatarg("commodity", Equip::types[t].name)));
			m_infoView->UpdateInfo();
		}
	}
};

class PersonalPage: public InfoViewPage {
public:
	PersonalPage(InfoView *v) : InfoViewPage(v) {};

	virtual void Show() {
		InfoViewPage::Show();
		m_infoView->ShowSpinner();
	}

	virtual void UpdateInfo() {
		Sint64 crime, fine;
		Polit::GetCrime(&crime, &fine);
		const float YSEP = Gui::Screen::GetFontHeight() * 1.5f;
		DeleteAllChildren();

		float ypos = 40.0f;
		Add((new Gui::Label(Lang::COMBAT_RATING))->Shadow(true), 40, ypos);
		Add(new Gui::Label(Pi::combatRating[ Pi::CombatRating(Pi::player->GetKillCount()) ]), 40, ypos+YSEP);

		ypos = 160.0f;
		Add((new Gui::Label(Lang::CRIMINAL_RECORD))->Shadow(true), 40, ypos);
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
	ShipInfoPage(InfoView *v) : InfoViewPage(v) {
		info1 = new Gui::Label("");
		info2 = new Gui::Label("");
		Add(info1, 40, 40);
		Add(info2, 250, 40);
		ShowAll();
	};

	virtual void Show() {
		InfoViewPage::Show();
		m_infoView->ShowSpinner();
	}

	virtual void UpdateInfo() {
		char buf[512];
		std::string col1, col2;
		const ShipType &stype = Pi::player->GetShipType();
		col1 = std::string(Lang::SHIP_INFORMATION_HEADER)+std::string(stype.name);
		col1 += "\n\n";
        col1 += std::string(Lang::HYPERDRIVE);
		col1 += ":\n\n";
        col1 += std::string(Lang::CAPACITY);
		col1 += ":\n";
        col1 += std::string(Lang::FREE);
		col1 += ":\n";
        col1 += std::string(Lang::USED);
		col1 += ":\n";
        col1 += std::string(Lang::TOTAL_WEIGHT);
		col1 += ":\n\n";
        col1 += std::string(Lang::FRONT_WEAPON);
		col1 += ":\n";
        col1 += std::string(Lang::REAR_WEAPON);
		col1 += ":\n\n";
        col1 += std::string(Lang::HYPERSPACE_RANGE);
        col1 += ":\n\n";
		
		col2 = "\n\n";

		Equip::Type e = Pi::player->m_equipment.Get(Equip::SLOT_ENGINE);
		col2 += std::string(Equip::types[e].name);

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
			col2 += std::string("\n\n")+Equip::types[e].name;
		} else {
			col2 += "\n\n";
            col2 += std::string(Lang::NO_MOUNTING);
		}
		if (numLasers >= 2) {
			e = Pi::player->m_equipment.Get(Equip::SLOT_LASER, 1);
			col2 += std::string("\n")+Equip::types[e].name;
		} else {
			col2 += "\n";
            col2 += std::string(Lang::NO_MOUNTING);
		}

		col2 += "\n\n";
		col2 += stringf(Lang::N_LIGHT_YEARS_N_MAX,
			formatarg("distance", stats->hyperspace_range),
			formatarg("maxdistance", stats->hyperspace_range_max));

		for (int i=Equip::FIRST_SHIPEQUIP; i<=Equip::LAST_SHIPEQUIP; i++) {
			Equip::Type t = Equip::Type(i) ;
			Equip::Slot s = Equip::types[t].slot;
			if ((s == Equip::SLOT_MISSILE) || (s == Equip::SLOT_ENGINE) || (s == Equip::SLOT_LASER)) continue;
			int num = Pi::player->m_equipment.Count(s, t);
			if (num == 1) {
				col1 += stringf("%0\n", Equip::types[t].name);
			} else if (num > 1) {
				// XXX this needs something more generic
				switch (t) {
					case Equip::SHIELD_GENERATOR:
						col1 += stringf(Lang::X_SHIELD_GENERATORS, formatarg ("quantity", int(num)));
						break;
					case Equip::PASSENGER_CABIN:
						col1 += stringf(Lang::X_PASSENGER_CABINS, formatarg ("quantity", int(num)));
						break;
					case Equip::UNOCCUPIED_CABIN:
						col1 += stringf(Lang::X_UNOCCUPIED_CABINS, formatarg ("quantity", int(num)));
						break;
					default:
						col1 += stringf("%0\n", Equip::types[t].name);
						break;
				}
				col1 += stringf("\n");
			} 
		}

		info1->SetText(col1);
		info2->SetText(col2);
		this->ResizeRequest();
	}
private:
	Gui::Label *info1, *info2;
};

InfoView::InfoView(): View(),
	m_spinner(0)
{
	SetTransparency(true);

	m_tabs = new Gui::Tabbed();

	InfoViewPage *page = new ShipInfoPage(this);
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label(Lang::SHIP_INFORMATION), page);

	page = new PersonalPage(this);
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label(Lang::REPUTATION), page);
	
	page = new CargoPage(this);
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label(Lang::CARGO), page);
	
	page = new MissionPage(this);
	m_pages.push_back(page);
	m_tabs->AddPage(new Gui::Label(Lang::MISSIONS), page);
	
	Add(m_tabs, 0, 0);

	m_doUpdate = true;
}

void InfoView::UpdateInfo()
{
	m_doUpdate = true;
}

void InfoView::Draw3D()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.0f,0.2f,0.4f,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void InfoView::Update()
{
	if (m_doUpdate) {
		for (std::list<InfoViewPage*>::iterator i = m_pages.begin(); i!=m_pages.end(); ++i) {
			(*i)->UpdateInfo();
		}
		m_doUpdate = false;
	}

	if (m_spinner) {
		if (m_showSpinner)
			m_spinner->Show();
		else
			m_spinner->Hide();
	}
}

void InfoView::NextPage()
{
	m_tabs->OnActivate();
}

void InfoView::OnSwitchTo()
{
	if (m_spinner)
		Remove(m_spinner);

	m_spinner = new ShipSpinnerWidget(*Pi::player->GetFlavour(), 320, 320);
	Add(m_spinner, 450, 50);
}
