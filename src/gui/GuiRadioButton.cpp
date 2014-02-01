// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"

static const float BUTTON_SIZE = 16.f;

namespace Gui {
RadioButton::RadioButton(Gui::RadioGroup *g)
{
	m_pressed = false;
	SetSize(BUTTON_SIZE, BUTTON_SIZE);
	if (g) g->Add(this);
}
RadioButton::~RadioButton()
{

}
bool RadioButton::OnMouseDown(MouseButtonEvent *e)
{
	if (e->button == SDL_BUTTON_LEFT) {
		onPress.emit();
		OnActivate();
		return false;
	} else
		return true;
}
void RadioButton::OnActivate()
{
//	if (!m_pressed) onSelect.emit();
	onSelect.emit();			// needs to emit even when pressed for time accel buttons
	m_pressed = true;			// does this break anything?
}
void RadioButton::GetSizeRequested(float size[2])
{
	size[0] = BUTTON_SIZE;
	size[1] = BUTTON_SIZE;
}

void RadioButton::Draw()
{
	PROFILE_SCOPED()
	float size[2];
	GetSize(size);
	if (m_pressed) {
		Theme::DrawIndent(size, Screen::alphaBlendState);
	} else {
		Theme::DrawOutdent(size, Screen::alphaBlendState);
	}
}
}
