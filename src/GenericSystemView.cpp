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
}


