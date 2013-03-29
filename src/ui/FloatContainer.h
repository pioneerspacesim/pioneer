// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_FLOATCONTAINER_H
#define UI_FLOATCONTAINER_H

#include "Container.h"

namespace UI {

class FloatContainer : public Container {
public:
	virtual void Layout();

	void AddWidget(Widget *w, const Point &pos, const Point &size);
	virtual void RemoveWidget(Widget *w);

private:
	virtual Point PreferredSize() { return Point(); }

	friend class Context;
	FloatContainer(Context *context) : Container(context) {}
};

}

#endif
