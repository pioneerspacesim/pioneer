#include "Checkbox.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

vector2f Checkbox::PreferredSize()
{
	return Skin::s_checkboxNormal.size;
}

void Checkbox::Layout()
{
	SetActiveArea(PreferredSize());
}

void Checkbox::Draw()
{
	if (m_checked)
		GetContext()->GetSkin().DrawCheckboxChecked(vector2f(0.0f), GetActiveArea());
	else
		GetContext()->GetSkin().DrawCheckboxNormal(vector2f(0.0f), GetActiveArea());
}

void Checkbox::HandleClick()
{
	m_checked = !m_checked;
}

}
