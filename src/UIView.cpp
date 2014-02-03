// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "UIView.h"
#include "Pi.h"
#include "ui/Context.h"
#include "gameui/Panel.h"

void UIView::OnSwitchTo()
{
	UI::VBox *box = Pi::ui->VBox();
	if (m_templateName)
		box->PackEnd(Pi::ui->CallTemplate(m_templateName));
	box->PackEnd(new GameUI::Panel(Pi::ui.Get()));

	Pi::ui->DropAllLayers();
	Pi::ui->GetTopLayer()->SetInnerWidget(box);
}

void UIView::OnSwitchFrom()
{
	Pi::ui->DropAllLayers();
	Pi::ui->Layout(); // UI does important things on layout, like updating keyboard shortcuts
}
