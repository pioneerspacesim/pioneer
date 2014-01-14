// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_WIDGETSET_H
#define UI_WIDGETSET_H

#include <vector>

struct lua_State;

namespace UI {

class Widget;

// Widget set. This class is a convenience to help add multiple widgets to a
// container in a single call
class WidgetSet {
public:
	inline WidgetSet() : numWidgets(0) { }

	inline WidgetSet(Widget *w0) : numWidgets(1) {
		widgets.resize(1);
		widgets[0] = w0;
	}
	inline WidgetSet(Widget *w0, Widget *w1) : numWidgets(2) {
		widgets.resize(2);
		widgets[0] = w0; widgets[1] = w1;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2) : numWidgets(3) {
		widgets.resize(3);
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3) : numWidgets(4) {
		widgets.resize(4);
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4) : numWidgets(5) {
		widgets.resize(5);
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5) : numWidgets(6) {
		widgets.resize(6);
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5, Widget *w6) : numWidgets(7) {
		widgets.resize(7);
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5; widgets[6] = w6;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5, Widget *w6, Widget *w7) : numWidgets(8) {
		widgets.resize(8);
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5; widgets[6] = w6; widgets[7] = w7;
	}

	inline WidgetSet(const std::vector<Widget*> &w) : widgets(w), numWidgets(w.size()) { }

	std::vector<Widget*> widgets;
	std::size_t numWidgets;
};

}

#endif
