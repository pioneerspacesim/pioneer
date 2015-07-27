// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"

namespace Gui {

static const float TOOLTIP_PADDING = 5.f;
static const float FADE_TIME_MS	   = 500.f;

ToolTip::ToolTip(Widget *owner, const char *text)
{
	m_owner = owner;
	m_layout = 0;
	SetText(text);
	m_createdTime = SDL_GetTicks();
}

ToolTip::ToolTip(Widget *owner, std::string &text)
{
	m_owner = owner;
	m_layout = 0;
	SetText(text.c_str());
	m_createdTime = SDL_GetTicks();
}

ToolTip::~ToolTip()
{
	delete m_layout;
}

void ToolTip::CalcSize()
{
	float size[2];
	m_layout->MeasureSize(400.0, size);
	size[0] += 2*TOOLTIP_PADDING;
	SetSize(size[0], size[1]);
	m_layout->Update(size[0]);
}

void ToolTip::SetText(const char *text)
{
	m_text = text;
	if (m_layout) delete m_layout;
	m_layout = new TextLayout(text);
	CalcSize();
}

void ToolTip::SetText(std::string &text)
{
	SetText(text.c_str());
}

void ToolTip::Draw()
{
	PROFILE_SCOPED()
	if (m_owner && !m_owner->IsVisible())
		return;

	float size[2];
	const int age = SDL_GetTicks() - m_createdTime;
	const float alpha = std::min(age / FADE_TIME_MS, 0.75f);

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	GetSize(size);
	const Color color(Color4f(0.2f, 0.2f, 0.6f, alpha));
	if(!m_background) {
		m_background.reset( new Graphics::Drawables::Rect(r, vector2f(0.f), vector2f(size[0], size[1]), color, Screen::alphaBlendState, false));
	}
	m_background->Update(vector2f(0.f), vector2f(size[0], size[1]), color);
	m_background->Draw(r);

	const vector3f outlineVts[] = {
		vector3f(size[0], 0, 0),
		vector3f(size[0], size[1], 0),
		vector3f(0, size[1], 0),
		vector3f(0, 0, 0)
	};
	const Color outlineColor(Color4f(0,0,.8f,alpha));
	m_outlines.SetData(2, &outlineVts[0], outlineColor);
	m_outlines.Draw(r, Screen::alphaBlendState, Graphics::LINE_LOOP);

	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);

	r->Translate(TOOLTIP_PADDING,0,0);
	m_layout->Render(size[0]-2*TOOLTIP_PADDING);
}

void ToolTip::GetSizeRequested(float size[2])
{
	m_layout->MeasureSize(size[0] - 2*TOOLTIP_PADDING, size);
	size[0] += 2*TOOLTIP_PADDING;
}

}
