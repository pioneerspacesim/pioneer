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

	// add padding
	const Skin::BorderedRectElement &elem(GetContext()->GetSkin().ButtonNormal());
	preferredSize = SizeAdd(preferredSize, Point(elem.paddingX*2, elem.paddingY*2));

	// grow to border size if necessary
	preferredSize.x = std::max(preferredSize.x, int(elem.borderWidth*2));
	preferredSize.y = std::max(preferredSize.y, int(elem.borderHeight*2));

	return preferredSize;
}

void Button::Layout()
{
	Widget *innerWidget = GetInnerWidget();

	const Skin::BorderedRectElement &elem(GetContext()->GetSkin().ButtonNormal());

	if (!innerWidget) {
		SetActiveArea(PreferredSize());
		return;
	}

	const Point innerSize = GetSize() - Point(elem.paddingX*2, elem.paddingY*2);
	SetWidgetDimensions(innerWidget, Point(elem.paddingX, elem.paddingY), innerWidget->CalcSize(innerSize));
	innerWidget->Layout();

	Point innerActiveArea(innerWidget->GetActiveArea());
	growToMinimum(innerActiveArea, GetContext()->GetSkin().ButtonMinInnerSize());

	SetActiveArea(innerActiveArea + Point(elem.paddingX*2, elem.paddingY*2));
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
