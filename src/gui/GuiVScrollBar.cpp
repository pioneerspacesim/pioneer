// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"

static const float SCROLLBAR_SIZE = 12.f;
static const float BORDER = 2.f;

namespace Gui {

ScrollBar::ScrollBar(bool isHoriz)
{
	m_isHoriz = isHoriz;
	m_isPressed = false;
	m_eventMask = EVENT_MOUSEDOWN;
	SetSize(SCROLLBAR_SIZE, SCROLLBAR_SIZE);
}

ScrollBar::~ScrollBar()
{
	if (_m_release) _m_release.disconnect();
	if (_m_motion) _m_motion.disconnect();
}

bool ScrollBar::OnMouseDown(MouseButtonEvent *e)
{
	float size[2];
	GetSize(size);
	if (e->button == SDL_BUTTON_LEFT) {
		m_isPressed = true;
		if (m_isHoriz) {
			m_adjustment->SetValue(e->x / float(size[0]));
		} else {
			m_adjustment->SetValue(e->y / float(size[1]));
		}
		_m_release = RawEvents::onMouseUp.connect(sigc::mem_fun(this, &ScrollBar::OnRawMouseUp));
		_m_motion = RawEvents::onMouseMotion.connect(sigc::mem_fun(this, &ScrollBar::OnRawMouseMotion));
	}
	else if (e->button == MouseButtonEvent::BUTTON_WHEELUP || e->button == MouseButtonEvent::BUTTON_WHEELDOWN) {
		float change = e->button == MouseButtonEvent::BUTTON_WHEELUP ? -0.1 : 0.1;
		float pos = m_adjustment->GetValue();
		m_adjustment->SetValue(Clamp(pos+change, 0.0f, 1.0f));
	}
	return false;
}

void ScrollBar::OnRawMouseUp(MouseButtonEvent *e) {
	if (e->button == SDL_BUTTON_LEFT) {
		m_isPressed = false;
		_m_release.disconnect();
		_m_motion.disconnect();
	}
}

void ScrollBar::OnRawMouseMotion(MouseMotionEvent *e)
{
	if (m_isPressed) {
		float pos[2];
		GetAbsolutePosition(pos);
		float size[2];
		GetSize(size);
		if (m_isHoriz) {
			m_adjustment->SetValue((e->x-pos[0]) / float(size[0]));
		} else {
			m_adjustment->SetValue((e->y-pos[1]) / float(size[1]));
		}
	}
}

void ScrollBar::Draw()
{
	PROFILE_SCOPED()
	float size[2]; GetSize(size);
	Theme::DrawIndent(size, Screen::alphaBlendState);
	float pos = m_adjustment->GetValue();
	vector3f lines[2];
	if (m_isHoriz) {
		lines[0] = vector3f(BORDER+(size[0]-2*BORDER)*pos, BORDER, 0.f);
		lines[1] = vector3f(BORDER+(size[0]-2*BORDER)*pos, size[1]-BORDER, 0.f);
	} else {
		lines[0] = vector3f(BORDER, BORDER+(size[1]-2*BORDER)*pos, 0.f);
		lines[1] = vector3f(size[0]-BORDER, BORDER+(size[1]-2*BORDER)*pos, 0.f);
	}
	Screen::GetRenderer()->DrawLines(2, &lines[0], Color::WHITE, Screen::alphaBlendState);
}

void ScrollBar::GetSizeRequested(float size[2])
{
	if (m_isHoriz) {
		// full X size, minimal Y size
		size[1] = SCROLLBAR_SIZE;
	} else {
		// full Y size, minimal X size
		size[0] = SCROLLBAR_SIZE;
	}
}

void ScrollBar::GetMinimumSize(float size[2])
{
	// who knows what the minimum size size is. odds are good that we're next
	// to a VScrollPortal which will provide a sane minimum size and the
	// container will sort out the rest
	size[0] = size[1] = SCROLLBAR_SIZE;
}

}
