#include "FloatContainer.h"

namespace UI {

vector2f FloatContainer::PreferredSize()
{
	return vector2f(FLT_MAX, FLT_MAX);
}

void FloatContainer::Layout()
{
	LayoutChildren();
}

void FloatContainer::AddWidget(Widget *w, const vector2f &pos, const vector2f &size)
{
	assert(!w->IsFloating());
	assert(!w->GetContainer());

	Container::AddWidget(w);
	w->SetFloating(true);

	w->SetDimensions(pos, size);

	w->Layout();
}

void FloatContainer::RemoveWidget(Widget *w)
{
	assert(w->IsFloating());
	assert(w->GetContainer());

	Container::RemoveWidget(w);
}

}
