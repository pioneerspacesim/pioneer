// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_BOX_H
#define UI_BOX_H

#include "Container.h"

namespace UI {

class Box : public Container {
protected:
	enum BoxOrientation {
		BOX_HORIZONTAL,
		BOX_VERTICAL
	};

	Box(Context *context, BoxOrientation orient, int spacing) : Container(context), m_orient(orient), m_spacing(spacing) {}

public:
	virtual Point PreferredSize();
	virtual void Layout();

	Box *PackStart(Widget *child);
	Box *PackStart(const WidgetSet &set);
	Box *PackEnd(Widget *child);
	Box *PackEnd(const WidgetSet &set);

	void Remove(Widget *child) { RemoveWidget(child); }
	void Clear();

protected:
	virtual void RemoveWidget(Widget *widget);

private:
	BoxOrientation m_orient;
	int m_spacing;

	struct Child {
		Child(Widget *_widget) : widget(_widget) {}
		Widget *widget;
		Point  contribSize;
		Point  size;
		Point  position;
	};

	std::list<Child> m_children;

	Point m_preferredSize;
	Uint32 m_minAllocation;
	Uint32 m_numVariable;
};

class VBox: public Box {
protected:
	friend class Context;
	VBox(Context *context, float spacing): Box(context, BOX_VERTICAL, spacing) {}
};

class HBox: public Box {
protected:
	friend class Context;
	HBox(Context *context, float spacing): Box(context, BOX_HORIZONTAL, spacing) {}
};

}

#endif
