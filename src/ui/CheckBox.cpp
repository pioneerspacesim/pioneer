#include "CheckBox.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

Point CheckBox::PreferredSize()
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
		GetContext()->GetSkin().DrawCheckBoxChecked(GetActiveOffset(), GetActiveArea());
	else
		GetContext()->GetSkin().DrawCheckBoxNormal(GetActiveOffset(), GetActiveArea());
}

void CheckBox::HandleClick()
{
	m_checked = !m_checked;
}

}
