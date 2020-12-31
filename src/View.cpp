// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "View.h"
#include "Pi.h"

View::View() :
	Gui::Fixed(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight() - 64))
{
}

View::~View()
{
	Gui::Screen::RemoveBaseWidget(this);
}

void View::Attach()
{
	OnSwitchTo();

	Gui::Screen::AddBaseWidget(this, 0, 0);

	ShowAll();
}

void View::Detach()
{
	Gui::Screen::RemoveBaseWidget(this);
	OnSwitchFrom();
}
