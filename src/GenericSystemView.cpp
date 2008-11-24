#include "Pi.h"
#include "GenericSystemView.h"
#include "SectorView.h"
#include "Sector.h"


GenericSystemView::GenericSystemView(): View()
{
	px = py = pidx = 0xdeadbeef;
	m_scannerLayout = new Gui::Fixed(360, 60);
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

	if (s && !s->IsSystem(px, py, pidx)) {
		s->GetPos(&px, &py, &pidx);
		Sector sec(px, py);
		Sector psec(playerLocSecX, playerLocSecY);
		const float dist = Sector::DistanceBetween(&sec, pidx, &psec, playerLocSysIdx);
		char buf[256];
		snprintf(buf, sizeof(buf), "Dist. %.2f light years", dist);

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

		m_systemName->SetText(sec.m_systems[pidx].name);
		m_distance->SetText(buf);
		m_starType->SetText(desc);
		m_shortDesc->SetText("Short description of system");

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

