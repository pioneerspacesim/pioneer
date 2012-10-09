// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "UIView.h"
#include "Pi.h"
#include "ui/Context.h"

void UIView::Update()
{
	Pi::ui->Update();
}

void UIView::Draw3D()
{
	Pi::ui->Draw();
}

void UIView::OnSwitchTo()
{
	Pi::ui->SetInnerWidget(Pi::ui->CallTemplate(m_templateName));
}

void UIView::OnSwitchFrom()
{
	Pi::ui->RemoveInnerWidget();
}
