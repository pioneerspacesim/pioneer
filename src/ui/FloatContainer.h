#ifndef _UI_FLOATCONTAINER_H
#define _UI_FLOATCONTAINER_H

#include "Container.h"

namespace UI {

class FloatContainer : public Container {
public:
	virtual void Layout();

	void AddWidget(Widget *w, const vector2f &pos, const vector2f &size);
	void RemoveWidget(Widget *w);

private:
	virtual vector2f PreferredSize() { return vector2f(); }

	friend class Context;
	FloatContainer(Context *context) : Container(context) {}
};

}

#endif
