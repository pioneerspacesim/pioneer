#include "Checkbox.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

// XXX this should probably be the font height
static const float MIN_CHECKBOX_INNER_SIZE = 16.0f;

vector2f Checkbox::PreferredSize()
{
	return vector2f(MIN_CHECKBOX_INNER_SIZE) + vector2f(Skin::s_buttonNormal.borderWidth*2); // XXX checkbox-specific button
}

void Checkbox::Layout()
{
	SetActiveArea(PreferredSize());
}

void Checkbox::Draw()
{
	if (m_checked)
		GetContext()->GetSkin().DrawButtonActive(vector2f(0.0f), GetActiveArea());
	else
		GetContext()->GetSkin().DrawButtonNormal(vector2f(0.0f), GetActiveArea());
}

void Checkbox::HandleClick()
{
	m_checked = !m_checked;
}

}
