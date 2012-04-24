#include "Button.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

// XXX this should probably be the font height
static const float MIN_BUTTON_INNER_SIZE = 16.0f;

static inline void growToMinimum(vector2f &v)
{
	if (v.x < MIN_BUTTON_INNER_SIZE || v.y < MIN_BUTTON_INNER_SIZE)
		v = vector2f(std::max(v.x,MIN_BUTTON_INNER_SIZE),std::max(v.y,MIN_BUTTON_INNER_SIZE));
}

vector2f Button::PreferredSize()
{
    // child's preferred size
	vector2f preferredSize(Single::PreferredSize());

	// grow to minimum size if necessary
	growToMinimum(preferredSize);

	// add borders
	preferredSize += vector2f(Skin::s_buttonNormal.borderWidth*2);

	return preferredSize;;
}

void Button::Layout()
{
	Widget *innerWidget = GetInnerWidget();

	if (!innerWidget) {
		SetActiveArea(vector2f(MIN_BUTTON_INNER_SIZE) + vector2f(Skin::s_buttonNormal.borderWidth*2));
		return;
	}

	const vector2f innerSize = GetSize() - vector2f(Skin::s_buttonNormal.borderWidth*2);
	SetWidgetDimensions(innerWidget, vector2f(Skin::s_buttonNormal.borderWidth), innerSize);
	innerWidget->Layout();

	vector2f innerActiveArea(innerWidget->GetActiveArea());
	growToMinimum(innerActiveArea);

	SetActiveArea(innerActiveArea + vector2f(Skin::s_buttonNormal.borderWidth*2));
}

void Button::Draw()
{
	if (IsMouseActive())
		GetContext()->GetSkin().DrawButtonActive(vector2f(0.0f), GetActiveArea());
	else
		GetContext()->GetSkin().DrawButtonNormal(vector2f(0.0f), GetActiveArea());

	Single::Draw();
}

}
