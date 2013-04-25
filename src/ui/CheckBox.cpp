// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CheckBox.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

Point CheckBox::PreferredSize()
{
	return GetContext()->GetSkin().CheckboxNormal().size;
}

void CheckBox::Layout()
{
	SetActiveArea(PreferredSize());
}

void CheckBox::Draw()
{
	if (m_checked) {
		if (IsDisabled())
			GetContext()->GetSkin().DrawCheckBoxCheckedDisabled(GetActiveOffset(), GetActiveArea());
		else if (IsMouseOver())
			GetContext()->GetSkin().DrawCheckBoxCheckedHover(GetActiveOffset(), GetActiveArea());
		else
			GetContext()->GetSkin().DrawCheckBoxCheckedNormal(GetActiveOffset(), GetActiveArea());
	} else {
		if (IsDisabled())
			GetContext()->GetSkin().DrawCheckBoxDisabled(GetActiveOffset(), GetActiveArea());
		else if (IsMouseOver())
			GetContext()->GetSkin().DrawCheckBoxHover(GetActiveOffset(), GetActiveArea());
		else
			GetContext()->GetSkin().DrawCheckBoxNormal(GetActiveOffset(), GetActiveArea());
	}
}

void CheckBox::Toggle()
{
	HandleClick();
}
const bool CheckBox::IsChecked()
{
	return m_checked;
}
void CheckBox::SetState(const bool state)
{
	if (state != m_checked)
	{
		HandleClick();
	}
}

void CheckBox::HandleClick()
{
	m_checked = !m_checked;
}

}
