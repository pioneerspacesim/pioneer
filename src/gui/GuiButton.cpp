// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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

Button::~Button()
{
	_m_release.disconnect();
	_m_kbrelease.disconnect();
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
		_m_release.disconnect();
		onRelease.emit();
		onClick.emit();
	}
	return false;
}

void Button::OnActivate()
{
	// activated by keyboard shortcut
	m_isPressed = true;
	_m_kbrelease = RawEvents::onKeyUp.connect(sigc::mem_fun(this, &Button::OnRawKeyUp));
	onPress.emit();
}

void Button::OnRawKeyUp(SDL_KeyboardEvent *e)
{
	if (e->keysym.sym == m_shortcut.sym) {
		m_isPressed = false;
		_m_kbrelease.disconnect();
		onRelease.emit();
		onClick.emit();
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

LabelButton::LabelButton(Label *label): Button()
{
	m_label = label;
	m_padding = 2.0;
	onSetSize.connect(sigc::mem_fun(this, &LabelButton::OnSetSize));
}

LabelButton::~LabelButton() { delete m_label; }

void LabelButton::GetSizeRequested(float size[2])
{
	m_label->GetSizeRequested(size);
	size[0] += 2*m_padding;
	//size[1] += 2*m_padding;
}

void LabelButton::Draw()
{
	float size[2];
	GetSize(size);
	//printf("%f,%f\n", size[0], size[1]);
	glColor3f(1,1,1);
	//Theme::DrawHollowRect(size);
	if (IsPressed()) {
		Theme::DrawIndent(size);
	} else {
		Theme::DrawOutdent(size);
	}
	glPushMatrix();
	glTranslatef(m_padding, m_padding*0.5, 0);
	m_label->Draw();
	glPopMatrix();
}

void LabelButton::OnSetSize()
{
	float size[2];
	GetSize(size);

	m_label->SetSize(size[0]-2*m_padding, size[1]);
}

}
