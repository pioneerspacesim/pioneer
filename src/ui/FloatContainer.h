#ifndef _UI_FLOATCONTAINER_H
#define _UI_FLOATCONTAINER_H

#include "Container.h"

namespace UI {

class FloatContainer : public Container {
public:
	virtual void Layout();

	void AddWidget(Widget *w, const Point &pos, const Point &size);
	void RemoveWidget(Widget *w);

private:
	virtual Point PreferredSize() { return Point(); }

	friend class Context;
	FloatContainer(Context *context) : Container(context) {}
};

}

#endif
