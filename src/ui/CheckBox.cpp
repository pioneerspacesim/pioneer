#include "CheckBox.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

vector2f CheckBox::PreferredSize()
{
	return Skin::s_checkboxNormal.size;
}

void CheckBox::Layout()
{
	SetActiveArea(PreferredSize());
}

void CheckBox::Draw()
{
	if (m_checked)
		GetContext()->GetSkin().DrawCheckBoxChecked(vector2f(0.0f), GetActiveArea());
	else
		GetContext()->GetSkin().DrawCheckBoxNormal(vector2f(0.0f), GetActiveArea());
}

void CheckBox::HandleClick()
{
	m_checked = !m_checked;
}

}
