#include "Pi.h"
#include "GenericSystemView.h"
#include "SectorView.h"
#include "Sector.h"
#include "Ship.h"
#include "Player.h"

GenericSystemView::GenericSystemView(): View()
{
	m_px = m_py = m_pidx = 0xdeadbeef;
	m_scannerLayout = new Gui::Fixed(450, 60);
	m_scannerLayout->SetTransparency(true);
	Gui::Screen::AddBaseWidget(m_scannerLayout, 140, Gui::Screen::GetHeight()-62);

	m_systemName = new Gui::Label("");
	m_systemName->SetColor(1,1,0);
	m_scannerLayout->Add(m_systemName, 40, 4);
	
	m_distance = new Gui::Label("");
	m_distance->SetColor(1,0,0);
	m_scannerLayout->Add(m_distance, 150, 4);

	m_starType = new Gui::Label("");
	m_starType->SetColor(1,0,1);
	m_scannerLayout->Add(m_starType, 22, 20);

	m_shortDesc = new Gui::Label("");
	m_shortDesc->SetColor(1,0,1);
	m_scannerLayout->Add(m_shortDesc, 5, 38);
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

