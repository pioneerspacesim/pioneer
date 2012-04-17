#include "Button.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

static const float MIN_BUTTON_INNER_SIZE = 24.0f;

Metrics Button::GetMetrics(const vector2f &hint)
{
	// reserve space for the borders
	Metrics metrics = Single::GetMetrics(hint - vector2f(Skin::s_buttonNormal.borderWidth*2));

	// grow to minimum size if necessary
	if (metrics.minimum.x < MIN_BUTTON_INNER_SIZE || metrics.minimum.y < MIN_BUTTON_INNER_SIZE)
		metrics.minimum = vector2f(std::max(metrics.minimum.x,MIN_BUTTON_INNER_SIZE),std::max(metrics.minimum.y,MIN_BUTTON_INNER_SIZE));
	if (metrics.ideal.x < MIN_BUTTON_INNER_SIZE || metrics.ideal.y < MIN_BUTTON_INNER_SIZE)
		metrics.ideal = vector2f(std::max(metrics.ideal.x,MIN_BUTTON_INNER_SIZE),std::max(metrics.ideal.y,MIN_BUTTON_INNER_SIZE));
	if (metrics.maximum.x < MIN_BUTTON_INNER_SIZE || metrics.maximum.y < MIN_BUTTON_INNER_SIZE)
		metrics.maximum = vector2f(std::max(metrics.maximum.x,MIN_BUTTON_INNER_SIZE),std::max(metrics.maximum.y,MIN_BUTTON_INNER_SIZE));

	// add borders
	metrics.minimum += vector2f(Skin::s_buttonNormal.borderWidth*2);
	metrics.ideal += vector2f(Skin::s_buttonNormal.borderWidth*2);
	metrics.maximum += vector2f(Skin::s_buttonNormal.borderWidth*2);

	return metrics;
}

void Button::Layout()
{
	if (!GetInnerWidget()) {
		SetActiveArea(vector2f(MIN_BUTTON_INNER_SIZE) + vector2f(Skin::s_buttonNormal.borderWidth*2));
		return;
	}

	const vector2f innerSize = GetSize() - vector2f(Skin::s_buttonNormal.borderWidth*2);
	SetWidgetDimensions(GetInnerWidget(), vector2f(Skin::s_buttonNormal.borderWidth), innerSize);
	GetInnerWidget()->Layout();

	SetActiveArea(GetInnerWidget()->GetActiveArea() + vector2f(Skin::s_buttonNormal.borderWidth*2));
}

void Button::Draw()
{
	if (m_active)
		GetContext()->GetSkin().DrawButtonActive(vector2f(0.0f), GetActiveArea());
	else
		GetContext()->GetSkin().DrawButtonNormal(vector2f(0.0f), GetActiveArea());

	Single::Draw();
}

void Button::Activate()
{
	m_active = true;
	Single::Activate();
}

void Button::Deactivate()
{
	m_active = false;
	Single::Deactivate();
}

}
