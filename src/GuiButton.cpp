#include "libs.h"
#include "Gui.h"

#define BUTTON_SIZE	16

namespace Gui {
Button::Button() 
{
	m_isPressed = false;
	m_eventMask = EVENT_MOUSEDOWN | EVENT_MOUSEUP;
	SetSize(BUTTON_SIZE, BUTTON_SIZE);
}
bool Button::OnMouseDown(MouseButtonEvent *e)
{
	if (e->button == 1) {
		m_isPressed = true;
		onPress.emit();
		// wait for mouse release, regardless of where on screen
		_m_release = RawEvents::onMouseUp.connect(sigc::mem_fun(this, &Button::OnRawMouseUp));
	}
	return false;
}

bool Button::OnMouseUp(MouseButtonEvent *e)
{
	if ((e->button == 1) && m_isPressed) {
		m_isPressed = false;
		onClick.emit();
	}
	return false;
}

void Button::OnActivate()
{
	// activated by keyboard shortcut
	m_isPressed = true;
	onPress.emit();
	_m_kbrelease = RawEvents::onKeyUp.connect(sigc::mem_fun(this, &Button::OnRawKeyUp));
}

void Button::OnRawKeyUp(SDL_KeyboardEvent *e)
{
	if (e->keysym.sym == m_shortcut.sym) {
		m_isPressed = false;
		onRelease.emit();
		onClick.emit();
		_m_kbrelease.disconnect();
	}
}

void Button::OnRawMouseUp(SDL_MouseButtonEvent *e)
{
	if (e->button == 1) {
		m_isPressed = false;
		_m_release.disconnect();
		onRelease.emit();
	}
}

void SolidButton::GetSizeRequested(float size[2])
{
	size[0] = size[1] = BUTTON_SIZE;
}

void TransparentButton::GetSizeRequested(float size[2])
{
	size[0] = size[1] = BUTTON_SIZE;
}

void SolidButton::Draw()
{
	glBegin(GL_QUADS);
		glColor3f(.6,.6,.6);
		glVertex2f(0,0);
		glVertex2f(15,0);
		glVertex2f(15,15);
		glVertex2f(0,15);
		
		glColor3fv(Color::bgShadow);
		glVertex2f(2,0);
		glVertex2f(15,0);
		glVertex2f(15,13);
		glVertex2f(2,13);
		
		glColor3fv(Color::bg);
		glVertex2f(2,2);
		glVertex2f(13,2);
		glVertex2f(13,13);
		glVertex2f(2,13);
	glEnd();

}
void TransparentButton::Draw()
{
	glColor3f(1,1,1);
	glBegin(GL_LINE_LOOP);
		glVertex2f(0,0);
		glVertex2f(15,0);
		glVertex2f(15,15);
		glVertex2f(0,15);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex2f(1,1);
		glVertex2f(14,1);
		glVertex2f(14,14);
		glVertex2f(1,14);
	glEnd();

}

}
