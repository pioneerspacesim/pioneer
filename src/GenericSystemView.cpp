#include "Pi.h"
#include "GenericSystemView.h"
#include "SectorView.h"
#include "Sector.h"
#include "Ship.h"
#include "Player.h"
#include "GalacticView.h"
#include "SystemView.h"
#include "SectorView.h"
#include "SystemInfoView.h"

const char *s_icons[] = {
	"icons/map_sector_view.png",
	"icons/map_sector_view_on.png",
	"icons/map_system_view.png",
	"icons/map_system_view_on.png",
	"icons/map_sysinfo_view.png",
	"icons/map_sysinfo_view_on.png",
	"icons/map_galactic_view.png",
	"icons/map_galactic_view_on.png",
};

GenericSystemView::GenericSystemView(enum MapView whichView): View()
{
	m_px = m_py = m_pidx = 0xdeadbeef;
	m_scannerLayout = new Gui::Fixed(450, 60);
	m_scannerLayout->SetTransparency(true);
	Gui::Screen::AddBaseWidget(m_scannerLayout, 140, Gui::Screen::GetHeight()-62);

	m_systemName = (new Gui::Label(""))->Color(1.0f, 1.0f, 0.0f);
	m_scannerLayout->Add(m_systemName, 40, 4);
	
	m_distance = (new Gui::Label(""))->Color(1.0f, 0.0f, 0.0f);
	m_scannerLayout->Add(m_distance, 150, 4);

	m_starType = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	m_scannerLayout->Add(m_starType, 22, 20);

	m_shortDesc = (new Gui::Label(""))->Color(1.0f, 0.0f, 1.0f);
	m_scannerLayout->Add(m_shortDesc, 5, 38);

	Gui::ImageButton *ib;
	
	ib = new Gui::ImageButton(s_icons[0 + (whichView==MAP_SECTOR ? 1 : 0)]);
	ib->SetShortcut(SDLK_F5, KMOD_NONE);
	ib->SetToolTip("Galaxy sector view");
	ib->onClick.connect(sigc::bind(sigc::ptr_fun(&GenericSystemView::OnClickView), MAP_SECTOR));
	m_rightButtonBar->Add(ib, 2, 2);
	
	ib = new Gui::ImageButton(s_icons[2 + (whichView==MAP_SYSTEM ? 1 : 0)]);
	ib->SetShortcut(SDLK_F6, KMOD_NONE);
	ib->SetToolTip("System orbit view");
	ib->onClick.connect(sigc::bind(sigc::ptr_fun(&GenericSystemView::OnClickView), MAP_SYSTEM));
	m_rightButtonBar->Add(ib, 34, 2);
	
	ib = new Gui::ImageButton(s_icons[4 + (whichView==MAP_INFO ? 1 : 0)]);
	ib->SetShortcut(SDLK_F7, KMOD_NONE);
	ib->SetToolTip("Star system information");
	ib->onClick.connect(sigc::bind(sigc::ptr_fun(&GenericSystemView::OnClickView), MAP_INFO));
	m_rightButtonBar->Add(ib, 66, 2);
	
	ib = new Gui::ImageButton(s_icons[6 + (whichView==MAP_GALACTIC ? 1 : 0)]);
	ib->SetShortcut(SDLK_F8, KMOD_NONE);
	ib->SetToolTip("Galactic view");
	ib->onClick.connect(sigc::bind(sigc::ptr_fun(&GenericSystemView::OnClickView), MAP_GALACTIC));
	m_rightButtonBar->Add(ib, 98, 2);
}

enum GenericSystemView::MapView GenericSystemView::currentView;

void GenericSystemView::OnClickView(enum MapView v)
{
	Pi::BoinkNoise();
	currentView = v;
	SwitchToCurrentView();
}

void GenericSystemView::SwitchToCurrentView()
{
	switch (currentView) {
		case MAP_SECTOR: Pi::SetView(Pi::sectorView); break;
		case MAP_SYSTEM: Pi::SetView(Pi::systemView); break;
		case MAP_INFO: Pi::SetView(Pi::systemInfoView); break;
		case MAP_GALACTIC: Pi::SetView(Pi::galacticView); break;
	}
}

void GenericSystemView::Draw3D()
{
	int playerLocSecX, playerLocSecY, playerLocSysIdx;
	Pi::currentSystem->GetPos(&playerLocSecX, &playerLocSecY, &playerLocSysIdx);
	StarSystem *s = Pi::GetSelectedSystem();

	if (s && !s->IsSystem(m_px, m_py, m_pidx)) {
		s->GetPos(&m_px, &m_py, &m_pidx);
		Sector sec(m_px, m_py);
		Sector psec(playerLocSecX, playerLocSecY);
		const float dist = Sector::DistanceBetween(&sec, m_pidx, &psec, playerLocSysIdx);
		char buf[256];
		SBodyPath sbody_path(m_px, m_py, m_pidx);
		int fuelRequired;
		bool canJump = Pi::player->CanHyperspaceTo(&sbody_path, fuelRequired);
		if (canJump) {
			snprintf(buf, sizeof(buf), "Dist. %.2f light years (fuel required: %dt)", dist, fuelRequired);
		} else if (fuelRequired) {
			snprintf(buf, sizeof(buf), "Dist. %.2f light years (insufficient fuel, required: %dt)", dist, fuelRequired);
		} else {
			snprintf(buf, sizeof(buf), "Dist. %.2f light years (out of range)", dist);
		}

		std::string desc;
		if (s->GetNumStars() == 4) {
			desc = "Quadruple system. ";
		} else if (s->GetNumStars() == 3) {
			desc = "Triple system. ";
		} else if (s->GetNumStars() == 2) {
			desc = "Binary system. ";
		} else {
			desc = s->rootBody->GetAstroDescription();
		}

		m_systemName->SetText(sec.m_systems[m_pidx].name);
		m_distance->SetText(buf);
		m_starType->SetText(desc);
		m_shortDesc->SetText(s->GetShortDescription());

		onSelectedSystemChanged.emit(s);
	}
}


void GenericSystemView::ShowAll()
{
	View::ShowAll();
	if (m_scannerLayout) m_scannerLayout->ShowAll();
}

void GenericSystemView::HideAll()
{
	View::HideAll();
	if (m_scannerLayout) m_scannerLayout->HideAll();
}

