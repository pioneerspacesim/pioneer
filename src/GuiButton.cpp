#include "libs.h"
#include "Gui.h"

#define BUTTON_SIZE	16

namespace Gui {
Button::Button() 
{
	m_isPressed = false;
	m_eventMask = EVENT_MOUSEDOWN | EVENT_MOUSEUP | EVENT_MOUSEMOTION;
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

void Button::OnRawMouseUp(MouseButtonEvent *e)
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
	float size[2];
	GetSize(size);
	if (IsPressed()) {
		Theme::DrawIndent(size);
	} else {
		Theme::DrawOutdent(size);
	}
}
void TransparentButton::Draw()
{
	float size[2];
	GetSize(size);
	glColor3f(1,1,1);
	Theme::DrawHollowRect(size);
}

}
