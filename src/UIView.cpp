// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "UIView.h"
#include "Pi.h"
#include "ui/Context.h"
#include "gameui/Panel.h"

void UIView::OnSwitchTo()
{
	UI::VBox *box = Pi::ui->VBox();
	UI::Expand *expander = Pi::ui->Expand();
	BuildUI(expander);
	box->PackEnd(expander);
	box->PackEnd(new GameUI::Panel(Pi::ui.Get()));

	Pi::ui->DropAllLayers();
	Pi::ui->GetTopLayer()->SetInnerWidget(box);
}

void UIView::OnSwitchFrom()
{
	Pi::ui->DropAllLayers();
	Pi::ui->Layout(); // UI does important things on layout, like updating keyboard shortcuts
}

void UIView::BuildUI(UI::Single *container) {
	UI::Widget *w = BuildTemplateUI();
	if (w) container->SetInnerWidget(w);
}

UI::Widget *UIView::BuildTemplateUI() {
	if (m_templateName)
		return Pi::ui->CallTemplate(m_templateName);
	else
		return nullptr;
}
