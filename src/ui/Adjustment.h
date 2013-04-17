// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_ADJUSTMENT_H
#define UI_ADJUSTMENT_H

#include "Container.h"

namespace UI {

class HBox;
class HSlider;

class Adjustment : public Container {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	Adjustment *SetInnerWidget(Widget *widget);
	void RemoveInnerWidget();
	Widget *GetInnerWidget() const { return m_innerWidget; }

	float GetScrollPosition() const;
	void SetScrollPosition(float v);
	UI::HSlider *GetSlider() const { return m_slider; }
	
	sigc::signal<void,float> onSliderChanged;

protected:
	friend class Context;
	Adjustment(Context *context) : Container(context), m_innerWidget(0), m_slider(0) {}

	virtual void RemoveWidget(Widget *widget);

private:
	Widget *m_innerWidget;
	HSlider *m_slider;

	sigc::connection m_onMouseWheelConn;

	void OnScroll(float value);
	bool OnMouseWheel(const MouseWheelEvent &event);
};

}

#endif
