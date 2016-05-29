// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SelectorBox.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

static inline void growToMinimum(Point &v, const int min)
{
	v.x = std::max(v.x, min);
	v.y = std::max(v.y, min);
}

static inline const Skin::BorderedRectElement &getElem(const Skin &skin, SelectorBox::Shape shape)
{
	switch (shape) {
		default: assert(0); // fall through
		case SelectorBox::RECT: return skin.SelectorRect();
		case SelectorBox::BRACKET: return skin.SelectorBracket();
	}
}

static inline int getMinSize(const Skin &skin, SelectorBox::Shape shape)
{
	switch (shape) {
		case SelectorBox::RECT: return skin.SelectorRectMinInnerSize(); break;
		case SelectorBox::BRACKET: return skin.SelectorBracketMinInnerSize(); break;
		default: return 0;
	}
}

Point SelectorBox::PreferredSize()
{
	// get sizing information from the skin
	const Skin &skin = GetContext()->GetSkin();
	const Skin::BorderedRectElement &elem = getElem(skin, m_shape);
	int minSize = getMinSize(skin, m_shape);

	// child's preferred size
	Point preferredSize(Single::PreferredSize());

	// grow to min size
	growToMinimum(preferredSize, minSize);

	// add padding
	preferredSize = SizeAdd(preferredSize, Point(elem.paddingX*2, elem.paddingY*2));

	// grow to border size if necessary
	preferredSize.x = std::max(preferredSize.x, int(elem.borderWidth*2));
	preferredSize.y = std::max(preferredSize.y, int(elem.borderHeight*2));

	return preferredSize;
}

void SelectorBox::Layout()
{
	Widget *innerWidget = GetInnerWidget();

	if (!innerWidget) {
		SetActiveArea(PreferredSize());
		return;
	}

	// get sizing information from the skin
	const Skin &skin = GetContext()->GetSkin();
	const Skin::BorderedRectElement &elem = getElem(skin, m_shape);
	int minSize = getMinSize(skin, m_shape);

	const Point innerSize = GetSize() - Point(elem.paddingX*2, elem.paddingY*2);
	SetWidgetDimensions(innerWidget, Point(elem.paddingX, elem.paddingY), innerWidget->CalcSize(innerSize));
	innerWidget->Layout();

	Point innerActiveArea(innerWidget->GetActiveArea());
	growToMinimum(innerActiveArea, minSize);
	SetActiveArea(innerActiveArea + Point(elem.paddingX*2, elem.paddingY*2));
}

void SelectorBox::Draw()
{
	if (IsShown()) {
		const Skin &skin = GetContext()->GetSkin();
		switch (m_shape) {
			case RECT: skin.DrawSelectorRect(GetActiveOffset(), GetActiveArea(), m_color);
			case BRACKET: skin.DrawSelectorBracket(GetActiveOffset(), GetActiveArea(), m_color);
		}
	}
	Single::Draw();
}

}
