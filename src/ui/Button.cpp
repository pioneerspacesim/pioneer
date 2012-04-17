#include "Button.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

// XXX STYLE
static const float buttonSize = 32.0f;

Metrics Button::GetMetrics(const vector2f &hint)
{
	Metrics metrics = Single::GetMetrics(hint - vector2f(buttonSize));

	metrics.minimum += vector2f(buttonSize);
	metrics.ideal += vector2f(buttonSize);
	metrics.maximum += vector2f(buttonSize);

	return metrics;
}

void Button::Update()
{
	SetActiveArea(vector2f(buttonSize));
	Single::Update();
}

void Button::Draw()
{
	vector2f drawSize(buttonSize);
	if (GetInnerWidget()) drawSize += GetInnerWidget()->GetSize();

	// XXX STYLE
	if (m_active)
		GetContext()->GetSkin().DrawButtonActive(vector2f(0.0f), drawSize);
	else
		GetContext()->GetSkin().DrawButtonNormal(vector2f(0.0f), drawSize);

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
