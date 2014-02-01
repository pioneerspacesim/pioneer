// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"

static const float METERBAR_PADDING    = 5.0f;
static const float METERBAR_BAR_HEIGHT = 8.0f;

namespace Gui {

MeterBar::MeterBar(float width, const char *label, const ::Color &graphCol)
{
	m_requestedWidth = width;
	m_barValue = 0;
	m_barColor = graphCol;
	m_label = new Gui::Label(label);
	Add(m_label, METERBAR_PADDING, METERBAR_PADDING + METERBAR_BAR_HEIGHT);
	m_label->Show();
}

void MeterBar::Draw()
{
	PROFILE_SCOPED()
	float size[2];
	GetSize(size);

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	Gui::Theme::DrawRoundEdgedRect(size, 5.0, Color(255,255,255,32), Screen::alphaBlendState);

	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);

	r->Translate(METERBAR_PADDING, METERBAR_PADDING, 0.0f);
	size[0] = m_barValue * (size[0] - 2.0f*METERBAR_PADDING);
	size[1] = METERBAR_BAR_HEIGHT;
	Gui::Theme::DrawRoundEdgedRect(size, 3.0f, m_barColor, Screen::alphaBlendState);

	Gui::Fixed::Draw();
}

void MeterBar::GetSizeRequested(float size[2])
{
	size[0] = m_requestedWidth;
	size[1] = METERBAR_PADDING*2.0f + METERBAR_BAR_HEIGHT + Gui::Screen::GetFontHeight();
}

}
