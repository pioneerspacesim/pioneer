// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "View.h"
#include "Pi.h"
#include "ShipCpanel.h"

View::View(): Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()-64)) {
	m_rightButtonBar = new Gui::Fixed(128, 26);
	m_rightButtonBar->SetBgColor(Color(160,160,160,255));

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

void View::Attach() {
	OnSwitchTo();

	const float w = float(Gui::Screen::GetWidth());
	const float h = float(Gui::Screen::GetHeight());

	Gui::Screen::AddBaseWidget(this, 0, 0);

	if (Pi::cpan) {
		Gui::Screen::AddBaseWidget(Pi::cpan, 0, h-80);
		Gui::Screen::AddBaseWidget(m_rightButtonBar, w-128, h-26);
		Gui::Screen::AddBaseWidget(m_rightRegion2, w-127, h-45);
		Gui::Screen::AddBaseWidget(m_rightRegion1, w-123, h-62);

		m_rightButtonBar->ShowAll();
		m_rightRegion2->ShowAll();
		m_rightRegion1->ShowAll();
	}

	ShowAll();
}

void View::Detach() {
	Gui::Screen::RemoveBaseWidget(m_rightButtonBar);
	Gui::Screen::RemoveBaseWidget(m_rightRegion2);
	Gui::Screen::RemoveBaseWidget(m_rightRegion1);
	Gui::Screen::RemoveBaseWidget(Pi::cpan);
	Gui::Screen::RemoveBaseWidget(this);
	if (Pi::cpan)
		Pi::cpan->ClearOverlay();

	OnSwitchFrom();
}
