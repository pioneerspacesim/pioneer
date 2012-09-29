// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _UI_WIDGETSET_H
#define _UI_WIDGETSET_H

#include <cassert>

struct lua_State;

namespace UI {

class Widget;

// Widget set. This class is a convenience to help add multiple widgets to a
// container in a single call
class WidgetSet {
public:
	inline WidgetSet() : numWidgets(0) { }
	inline WidgetSet(Widget *w0) : numWidgets(1) {
		widgets[0] = w0;
	}
	inline WidgetSet(Widget *w0, Widget *w1) : numWidgets(2) {
		widgets[0] = w0; widgets[1] = w1;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2) : numWidgets(3) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3) : numWidgets(4) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4) : numWidgets(5) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5) : numWidgets(6) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5, Widget *w6) : numWidgets(7) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5; widgets[6] = w6;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5, Widget *w6, Widget *w7) : numWidgets(8) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5; widgets[6] = w6; widgets[7] = w7;
	}

	inline WidgetSet(Widget *w[], int n) : numWidgets(n) {
		assert(n >= 0 && n <= 8);
		for (int i = 0; i < n; i++) widgets[i] = w[i];
	}

	int numWidgets;
	Widget *widgets[8];
};

}

#endif
