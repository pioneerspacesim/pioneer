#include "Margin.h"

namespace UI {

Point Margin::PreferredSize()
{
	if (!GetInnerWidget()) return Point(m_margin*2.0f);
	return GetInnerWidget()->PreferredSize() + Point(m_margin*2.0f);
}

void Margin::Layout()
{
	if (!GetInnerWidget()) return;
	SetWidgetDimensions(GetInnerWidget(), Point(m_margin), Point(std::max(GetSize().x-m_margin*2.0f,0.0f), std::max(GetSize().y-m_margin*2.0f,0.0f)));
	GetInnerWidget()->Layout();
}

}
