// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Button.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

// XXX this should probably be the font height
static const int MIN_BUTTON_INNER_SIZE = 16;

static inline void growToMinimum(Point &v)
{
	if (v.x < MIN_BUTTON_INNER_SIZE || v.y < MIN_BUTTON_INNER_SIZE)
		v = Point(std::max(v.x,MIN_BUTTON_INNER_SIZE),std::max(v.y,MIN_BUTTON_INNER_SIZE));
}

Point Button::PreferredSize()
{
    // child's preferred size
	Point preferredSize(Single::PreferredSize());

	// grow to minimum size if necessary
	growToMinimum(preferredSize);

	// add borders
	preferredSize += Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2);

	return preferredSize;;
}

void Button::Layout()
{
	Widget *innerWidget = GetInnerWidget();

	if (!innerWidget) {
		SetActiveArea(Point(MIN_BUTTON_INNER_SIZE) + Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2));
		return;
	}

	const Point innerSize = GetSize() - Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2);
	SetWidgetDimensions(innerWidget, Point(GetContext()->GetSkin().ButtonNormal().borderWidth), innerSize);
	innerWidget->Layout();

	Point innerActiveArea(innerWidget->GetActiveArea());
	growToMinimum(innerActiveArea);

	SetActiveArea(innerActiveArea + Point(GetContext()->GetSkin().ButtonNormal().borderWidth*2));
}

void Button::Draw()
{
	if (IsMouseActive())
		GetContext()->GetSkin().DrawButtonActive(GetActiveOffset(), GetActiveArea());
	else if (IsMouseOver())
		GetContext()->GetSkin().DrawButtonHover(GetActiveOffset(), GetActiveArea());
	else
		GetContext()->GetSkin().DrawButtonNormal(GetActiveOffset(), GetActiveArea());

	Single::Draw();
}

}
