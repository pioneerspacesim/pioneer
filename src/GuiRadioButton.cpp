#include "libs.h"
#include "Gui.h"

#define BUTTON_SIZE	16

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
	onPress.emit();
	OnActivate();
	return false;
}
void RadioButton::OnActivate()
{
	if (!m_pressed) onSelect.emit();
	m_pressed = true;
}
void RadioButton::GetSizeRequested(float size[2])
{
	size[0] = BUTTON_SIZE;
	size[1] = BUTTON_SIZE;
}

void RadioButton::Draw()
{
	float size[2];
	GetSize(size);
	if (m_pressed) {
		Theme::DrawIndent(size);
	} else {
		Theme::DrawOutdent(size);
	}
}
}
