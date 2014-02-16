// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
	const Skin::BorderedRectElement &elem(GetContext()->GetSkin().ButtonNormal());
	return SizeAdd(preferredSize, Point(elem.borderWidth*2, elem.borderHeight*2));
}

void Button::Layout()
{
	Widget *innerWidget = GetInnerWidget();

	const Skin::BorderedRectElement &elem(GetContext()->GetSkin().ButtonNormal());

	if (!innerWidget) {
		SetActiveArea(Point(GetContext()->GetSkin().ButtonMinInnerSize()) + Point(elem.borderWidth*2, elem.borderHeight*2));
		return;
	}

	const Point innerSize = GetSize() - Point(elem.borderWidth*2, elem.borderHeight*2);
	SetWidgetDimensions(innerWidget, Point(elem.borderWidth, elem.borderHeight), innerWidget->CalcSize(innerSize));
	innerWidget->Layout();

	Point innerActiveArea(innerWidget->GetActiveArea());
	growToMinimum(innerActiveArea, GetContext()->GetSkin().ButtonMinInnerSize());

	SetActiveArea(innerActiveArea + Point(elem.borderWidth*2, elem.borderHeight*2));
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
