#include "View.h"
#include "Pi.h"
#include "ShipCpanel.h"

View::View(): Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()-64)) {
	m_rightButtonBar = new Gui::Fixed(128, 26);
	m_rightButtonBar->SetBgColor(.65f, .65f, .65f, 1.0f);

	m_rightRegion2 = new Gui::Fixed(126, 17);
	m_rightRegion2->SetTransparency(true);

	m_rightRegion1 = new Gui::Fixed(122, 17);
	m_rightRegion1->SetTransparency(true);
}

View::~View() {
	Gui::Screen::RemoveBaseWidget(m_rightButtonBar);
	Gui::Screen::RemoveBaseWidget(m_rightRegion2);
	Gui::Screen::RemoveBaseWidget(m_rightRegion1);
	Gui::Screen::RemoveBaseWidget(Pi::cpan);
	Gui::Screen::RemoveBaseWidget(this);
	delete m_rightButtonBar;
	delete m_rightRegion2;
	delete m_rightRegion1;
}

void View::ShowAll() {
	Gui::Screen::AddBaseWidget(this, 0, 0);

	if (Pi::game) {
		Gui::Screen::AddBaseWidget(Pi::cpan, 0, Gui::Screen::GetHeight()-80);
		Gui::Screen::AddBaseWidget(m_rightButtonBar, Gui::Screen::GetWidth()-128, Gui::Screen::GetHeight()-26);
		Gui::Screen::AddBaseWidget(m_rightRegion2, Gui::Screen::GetWidth()-127, Gui::Screen::GetHeight()-45);
		Gui::Screen::AddBaseWidget(m_rightRegion1, Gui::Screen::GetWidth()-123, Gui::Screen::GetHeight()-62);

		m_rightButtonBar->ShowAll();
		m_rightRegion2->ShowAll();
		m_rightRegion1->ShowAll();
	}

	Gui::Fixed::ShowAll();
}

void View::HideAll() {
	m_rightButtonBar->HideAll();
	m_rightRegion2->HideAll();
	m_rightRegion1->HideAll();
	Gui::Fixed::HideAll();

	Gui::Screen::RemoveBaseWidget(m_rightButtonBar);
	Gui::Screen::RemoveBaseWidget(m_rightRegion2);
	Gui::Screen::RemoveBaseWidget(m_rightRegion1);
	Gui::Screen::RemoveBaseWidget(Pi::cpan);
	Gui::Screen::RemoveBaseWidget(this);
}
