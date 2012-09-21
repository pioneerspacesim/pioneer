#ifndef _UI_SCROLLER_H
#define _UI_SCROLLER_H

#include "Container.h"

namespace UI {

class HBox;
class VSlider;

class Scroller : public Container {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	Scroller *SetInnerWidget(Widget *widget);
	void RemoveInnerWidget();
	Widget *GetInnerWidget() const { return m_innerWidget; }

protected:
	friend class Context;
	Scroller(Context *context);

private:
	Widget *m_innerWidget;
	VSlider *m_slider;

	sigc::connection m_onMouseWheelConn;

	void OnScroll(float value);
	bool OnMouseWheel(const MouseWheelEvent &event);
};

}

#endif
