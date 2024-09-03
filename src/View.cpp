// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "View.h"
#include "Pi.h"

View::View() :
	m_renderer(nullptr)
{
}

View::~View()
{
}

void View::Attach()
{
	OnSwitchTo();
}

void View::Detach()
{
	OnSwitchFrom();
}
