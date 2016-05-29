#include "OverlayStack.h"

namespace UI {

Point OverlayStack::PreferredSize()
{
	Point sz(0, 0);
	for (auto it : GetWidgets()) {
		Point wantSize = it->CalcLayoutContribution();
		if (wantSize.x != SIZE_EXPAND) { sz.x = std::max(sz.x, wantSize.x); }
		if (wantSize.y != SIZE_EXPAND) { sz.y = std::max(sz.y, wantSize.y); }
	}
	return sz;
}

void OverlayStack::Layout()
{
	Point sz = GetSize();
	for (auto it : GetWidgets()) {
		SetWidgetDimensions(it.Get(), Point(), it->CalcSize(sz));
		it->Layout();
	}
}

OverlayStack *OverlayStack::AddLayer(Widget *widget)
{
	AddWidget(widget);
	return this;
}

void OverlayStack::PopLayer()
{
	if (GetNumWidgets() < 1) { return; }
	const auto it = (GetWidgets().end() - 1);
	RemoveWidget(it->Get());
}

void OverlayStack::Clear()
{
	RemoveAllWidgets();
}

}
