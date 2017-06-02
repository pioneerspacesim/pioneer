// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"

static const float METERBAR_PADDING    = 5.0f;
static const float METERBAR_BAR_HEIGHT = 8.0f;
static const Color OUTER_COLOUR(255,255,255,32);

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
	float fsize[2];
	GetSize(fsize);
	vector2f size(fsize[0], fsize[1]);

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	if(!m_outer)
		m_outer.reset( new Graphics::Drawables::RoundEdgedRect(r, size, 5.0f, OUTER_COLOUR, Screen::alphaBlendState, false) );
	else
		m_outer->Update(size, 5.0f, OUTER_COLOUR);

	m_outer->Draw(r);

	// draw inner bar
	{
		Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);
		r->Translate(METERBAR_PADDING, METERBAR_PADDING, 0.0f);
		size.x = m_barValue * (size.x - 2.0f*METERBAR_PADDING);
		size.y = METERBAR_BAR_HEIGHT;
		if(!m_inner)
			m_inner.reset( new Graphics::Drawables::RoundEdgedRect(r, size, 3.0f, m_barColor, Screen::alphaBlendState, false) );
		else
			m_inner->Update(size, 3.0f, m_barColor);

		m_inner->Draw(r);
	}
	Gui::Fixed::Draw();
}

void MeterBar::GetSizeRequested(float size[2])
{
	size[0] = m_requestedWidth;
	size[1] = METERBAR_PADDING*2.0f + METERBAR_BAR_HEIGHT + Gui::Screen::GetFontHeight();
}

}
