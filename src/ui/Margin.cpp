// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Margin.h"

namespace UI {

Point Margin::PreferredSize()
{
	if (!GetInnerWidget()) return Point(m_margin*2.0f);
	return SizeAdd(GetInnerWidget()->PreferredSize(), Point(m_margin*2.0f));
}

void Margin::Layout()
{
	if (!GetInnerWidget()) return;
	SetWidgetDimensions(GetInnerWidget(), Point(m_margin), Point(std::max(GetSize().x-m_margin*2.0f,0.0f), std::max(GetSize().y-m_margin*2.0f,0.0f)));
	GetInnerWidget()->Layout();
}

}
