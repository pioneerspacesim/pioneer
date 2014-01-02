// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_LAYER_H
#define UI_LAYER_H

#include "Container.h"

namespace UI {

class Layer : public Container {
public:
	virtual void Layout();

	Layer *SetInnerWidget(Widget *w, const Point &pos, const Point &size);
	Layer *SetInnerWidget(Widget *w) { return SetInnerWidget(w, GetPosition(), GetSize()); }
	virtual void RemoveInnerWidget();
	Widget *GetInnerWidget() const { return m_widget.Get(); }

private:
	virtual Point PreferredSize() { return Point(); }

	friend class Context;
	Layer(Context *context) : Container(context) {}

	RefCountedPtr<Widget> m_widget;
};

}

#endif
