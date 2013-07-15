// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Button.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

static inline void growToMinimum(Point &v, const int min)
{
	if (v.x < min || v.y < min)
		v = Point(std::max(v.x,min),std::max(v.y,min));
}

Point Button::PreferredSize()
{
	// child's preferred size
	Point preferredSize(Single::PreferredSize());

	// grow to minimum size if necessary
	growToMinimum(preferredSize, GetContext()->GetSkin().ButtonMinInnerSize());

	// add borders
	return SizeAdd(preferredSize, Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2));
}

void Button::Layout()
{
	Widget *innerWidget = GetInnerWidget();

	if (!innerWidget) {
		SetActiveArea(Point(GetContext()->GetSkin().ButtonMinInnerSize()) + Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2));
		return;
	}

	const Point innerSize = GetSize() - Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2);
	SetWidgetDimensions(innerWidget, Point(GetContext()->GetSkin().ButtonNormal().borderWidth), innerWidget->CalcSize(innerSize));
	innerWidget->Layout();

	Point innerActiveArea(innerWidget->GetActiveArea());
	growToMinimum(innerActiveArea, GetContext()->GetSkin().ButtonMinInnerSize());

	SetActiveArea(innerActiveArea + Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2));
}

void Button::Draw()
{
	if (IsDisabled())
		GetContext()->GetSkin().DrawButtonDisabled(GetActiveOffset(), GetActiveArea());
	else if (IsMouseActive())
		GetContext()->GetSkin().DrawButtonActive(GetActiveOffset(), GetActiveArea());
	else if (IsMouseOver())
		GetContext()->GetSkin().DrawButtonHover(GetActiveOffset(), GetActiveArea());
	else
		GetContext()->GetSkin().DrawButtonNormal(GetActiveOffset(), GetActiveArea());

	Single::Draw();
}

}
