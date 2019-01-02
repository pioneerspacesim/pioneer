#include "OverlayStack.h"

namespace UI {

	Point OverlayStack::PreferredSize()
	{
		Point sz(0, 0);
		for (auto it : Container::GetWidgets()) {
			sz = Point::Max(sz, it->CalcLayoutContribution());
		}
		return sz;
	}

	void OverlayStack::Layout()
	{
		Point sz = GetSize();
		for (auto it : Container::GetWidgets()) {
			SetWidgetDimensions(it.Get(), Point(), it->CalcSize(sz));
			it->Layout();
		}
	}

	OverlayStack *OverlayStack::AddLayer(Widget *widget)
	{
		AddWidget(widget);
		return this;
	}

	void OverlayStack::Clear()
	{
		RemoveAllWidgets();
	}

} // namespace UI
