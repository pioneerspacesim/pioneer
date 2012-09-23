#include "Button.h"
#include "Context.h"
#include "Skin.h"

namespace UI {

// XXX this should probably be the font height
static const int MIN_BUTTON_INNER_SIZE = 16;

static inline void growToMinimum(Point &v)
{
	if (v.x < MIN_BUTTON_INNER_SIZE || v.y < MIN_BUTTON_INNER_SIZE)
		v = Point(std::max(v.x,MIN_BUTTON_INNER_SIZE),std::max(v.y,MIN_BUTTON_INNER_SIZE));
}

Point Button::PreferredSize()
{
    // child's preferred size
	Point preferredSize(Single::PreferredSize());

	// grow to minimum size if necessary
	growToMinimum(preferredSize);

	// add borders
	preferredSize += Point(Skin::s_buttonNormal.borderWidth*2);

	return preferredSize;;
}

void Button::Layout()
{
	Widget *innerWidget = GetInnerWidget();

	if (!innerWidget) {
		SetActiveArea(Point(MIN_BUTTON_INNER_SIZE) + Point(Skin::s_buttonNormal.borderWidth*2));
		return;
	}

	const Point innerSize = GetSize() - Point(Skin::s_buttonNormal.borderWidth*2);
	SetWidgetDimensions(innerWidget, Point(Skin::s_buttonNormal.borderWidth), innerSize);
	innerWidget->Layout();

	Point innerActiveArea(innerWidget->GetActiveArea());
	growToMinimum(innerActiveArea);

	SetActiveArea(innerActiveArea + Point(Skin::s_buttonNormal.borderWidth*2));
}

void Button::Draw()
{
	if (IsMouseActive())
		GetContext()->GetSkin().DrawButtonActive(GetActiveOffset(), GetActiveArea());
	else
		GetContext()->GetSkin().DrawButtonNormal(GetActiveOffset(), GetActiveArea());

	Single::Draw();
}

}
