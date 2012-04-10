#include "Margin.h"

namespace UI {

Metrics Margin::GetMetrics(const vector2f &hint)
{
	if (!GetInnerWidget()) return Metrics(m_margin*2.0f,m_margin*2.0f);
	Metrics metrics = GetInnerWidget()->GetMetrics(vector2f(std::max(hint.x-m_margin*2.0f,0.0f), std::max(hint.y-m_margin*2.0f,0.0f)));
	return Metrics(
		metrics.ideal + vector2f(m_margin*2.0f),
		metrics.minimum + vector2f(m_margin*2.0f),
		vector2f(std::min(metrics.maximum.x+m_margin*2.0f,FLT_MAX), std::min(metrics.maximum.y+m_margin*2.0f,FLT_MAX))
	);
}

void Margin::Layout()
{
	if (!GetInnerWidget()) return;
	SetWidgetDimensions(GetInnerWidget(), GetPosition() + vector2f(m_margin,m_margin), vector2f(std::max(GetSize().x-m_margin*2.0f,0.0f), std::max(GetSize().y-m_margin*2.0f,0.0f)));
	GetInnerWidget()->Layout();
}

}
