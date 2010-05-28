#include "libs.h"
#include "Gui.h"

#define SCROLLBAR_SIZE	12
#define BORDER	2

namespace Gui {

VScrollBar::VScrollBar()
{
	m_isPressed = false;
	m_eventMask = EVENT_MOUSEDOWN;
	SetSize(SCROLLBAR_SIZE, SCROLLBAR_SIZE);
}

VScrollBar::~VScrollBar()
{
	if (_m_release) _m_release.disconnect();
	if (_m_motion) _m_motion.disconnect();
}

bool VScrollBar::OnMouseDown(MouseButtonEvent *e)
{
	float size[2];
	GetSize(size);
	if (e->button == 1) {
		m_isPressed = true;
		m_adjustment->SetValue(e->y / (float)size[1]);
		_m_release = RawEvents::onMouseUp.connect(sigc::mem_fun(this, &VScrollBar::OnRawMouseUp));
		_m_motion = RawEvents::onMouseMotion.connect(sigc::mem_fun(this, &VScrollBar::OnRawMouseMotion));
	}
	return false;
}

void VScrollBar::OnRawMouseUp(MouseButtonEvent *e) {
	if (e->button == 1) {
		m_isPressed = false;
		_m_release.disconnect();
		_m_motion.disconnect();
	}
}

void VScrollBar::OnRawMouseMotion(MouseMotionEvent *e)
{
	if (m_isPressed) {
		float pos[2];
		GetAbsolutePosition(pos);
		float size[2];
		GetSize(size);
		m_adjustment->SetValue((e->y-pos[1]) / (float)size[1]);
	}
}

void VScrollBar::Draw()
{
	float size[2]; GetSize(size);
	Theme::DrawIndent(size);
	float pos = m_adjustment->GetValue();
	glColor3f(1,1,1);
	glBegin(GL_LINES);
		glVertex2f(BORDER, BORDER+(size[1]-2*BORDER)*pos);
		glVertex2f(size[0]-BORDER, BORDER+(size[1]-2*BORDER)*pos);
	glEnd();
}

void VScrollBar::GetSizeRequested(float size[2])
{
	size[0] = SCROLLBAR_SIZE;
	// full Y size
}

}

