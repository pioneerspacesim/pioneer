// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "View.h"
#include "Pi.h"
#include "pigui/LuaPiGui.h"

View::View(const std::string &name) :
	m_renderer(nullptr),
	m_handlerName(name)
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

void View::DrawPiGui()
{
	PiGui::RunHandler(Pi::GetFrameTime(), m_handlerName);
}
