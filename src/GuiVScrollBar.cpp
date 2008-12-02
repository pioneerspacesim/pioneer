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

void VScrollBar::OnRawMouseUp(SDL_MouseButtonEvent *e) {
	if (e->button == 1) {
		m_isPressed = false;
		_m_release.disconnect();
		_m_motion.disconnect();
	}
}

void VScrollBar::OnRawMouseMotion(SDL_MouseMotionEvent *e)
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
	glColor3f(1,1,0);
	glBegin(GL_QUADS);
		glColor3f(.6,.6,.6);
		glVertex2f(0, size[1]);
		glVertex2f(size[0], size[1]);
		glVertex2f(size[0], 0);
		glVertex2f(0, 0);

		glColor3fv(Color::bgShadow);
		glVertex2f(0, size[1]-BORDER);
		glVertex2f(size[0]-BORDER, size[1]-BORDER);
		glVertex2f(size[0]-BORDER, 0);
		glVertex2f(0, 0);

		glColor3fv(Color::bg);
		glVertex2f(BORDER, size[1]-BORDER);
		glVertex2f(size[0]-BORDER, size[1]-BORDER);
		glVertex2f(size[0]-BORDER, BORDER);
		glVertex2f(BORDER, BORDER);

	glEnd();
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

