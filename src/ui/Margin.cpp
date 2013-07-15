// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Margin.h"

namespace UI {

Point Margin::PreferredSize()
{
	if (!GetInnerWidget()) return Point(m_margin*2.0f);
	return SizeAdd(GetInnerWidget()->CalcLayoutContribution(), Point(m_margin*2.0f));
}

void Margin::Layout()
{
	if (!GetInnerWidget()) return;

	const Point size = GetSize();

	Point innerPos, innerSize;
	switch (m_direction) {
		case ALL:
			innerPos = Point(m_margin);
			innerSize = Point(std::max(size.x-m_margin*2,0), std::max(size.y-m_margin*2,0));
			break;

		case HORIZONTAL:
			innerPos = Point(m_margin,0);
			innerSize = Point(std::max(size.x-m_margin*2,0), size.y);
			break;

		case VERTICAL:
			innerPos = Point(0,m_margin);
			innerSize = Point(size.x, std::max(size.y-m_margin*2,0));
			break;

		case LEFT:
			innerPos = Point(m_margin,0);
			innerSize = Point(std::max(size.x-m_margin,0), size.y);
			break;

		case RIGHT:
			innerPos = Point(0,0);
			innerSize = Point(std::max(size.x-m_margin,0), size.y);
			break;

		case TOP:
			innerPos = Point(0,m_margin);
			innerSize = Point(size.x, std::max(size.y-m_margin,0));
			break;

		case BOTTOM:
			innerPos = Point(0,0);
			innerSize = Point(size.x, std::max(size.y-m_margin,0));
			break;
	}


	SetWidgetDimensions(GetInnerWidget(), innerPos, GetInnerWidget()->CalcSize(innerSize));

	GetInnerWidget()->Layout();
}

}
