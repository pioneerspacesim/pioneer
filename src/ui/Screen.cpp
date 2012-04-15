#include "Screen.h"

namespace UI {

Screen::Screen(Context *context, int width, int height) : Single(context),
	m_eventDispatcher(this),
	m_width(float(width)),
	m_height(float(height))
{
	SetSize(vector2f(m_width,m_height));
}

Metrics Screen::GetMetrics(const vector2f &hint)
{
	return Metrics(vector2f(m_width,m_height), vector2f(m_width,m_height), vector2f(m_width,m_height));
}

}
