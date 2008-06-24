#include "Pi.h"
#include "GenericSystemView.h"
#include "SectorView.h"


GenericSystemView::GenericSystemView(): View()
{
	px = py = pidx = 0xdeadbeef;
	m_scannerLayout = new Gui::Fixed(140, 2, 360, 60);
	m_scannerLayout->SetTransparency(true);

	m_systemName = new Gui::Label("");
	m_systemName->SetColor(1,1,0);
	m_scannerLayout->Add(m_systemName, 40, 44);
	
	m_distance = new Gui::Label("");
	m_distance->SetColor(1,0,0);
	m_scannerLayout->Add(m_distance, 150, 44);

	m_starType = new Gui::Label("");
	m_starType->SetColor(1,0,1);
	m_scannerLayout->Add(m_starType, 22, 26);

	m_shortDesc = new Gui::Label("");
	m_shortDesc->SetColor(1,0,1);
	m_scannerLayout->Add(m_shortDesc, 5, 8);
}
	
void GenericSystemView::Draw3D()
{
	StarSystem *s = Pi::GetSelectedSystem();

	if (s && !s->IsSystem(px, py, pidx)) {
		s->GetPos(&px, &py, &pidx);

		m_systemName->SetText(s->rootBody->name);
		m_distance->SetText("Dist. XX.XX light years");
		m_starType->SetText(s->rootBody->GetAstroDescription());
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

