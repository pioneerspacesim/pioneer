#include "Margin.h"

namespace UI {

vector2f Margin::PreferredSize()
{
	if (!GetInnerWidget()) return vector2f(m_margin*2.0f);
	return GetInnerWidget()->PreferredSize() + vector2f(m_margin*2.0f);
}

void Margin::Layout()
{
	if (!GetInnerWidget()) return;
	SetWidgetDimensions(GetInnerWidget(), vector2f(m_margin), vector2f(std::max(GetSize().x-m_margin*2.0f,0.0f), std::max(GetSize().y-m_margin*2.0f,0.0f)));
	GetInnerWidget()->Layout();
}

}
