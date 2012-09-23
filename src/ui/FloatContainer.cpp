#include "FloatContainer.h"

namespace UI {

void FloatContainer::Layout()
{
	LayoutChildren();
}

void FloatContainer::AddWidget(Widget *w, const Point &pos, const Point &size)
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
