// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
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

	Box(Context *context, BoxOrientation orient, float spacing) : Container(context), m_orient(orient), m_spacing(spacing), m_countExpanded(0) {}

public:
	virtual Point PreferredSize();
	virtual void Layout();

	enum BoxFlags { // <enum scope='UI::Box' name=UIBoxFlags prefix=BOX_>
		BOX_EXPAND = 0x1,   // if true, give this child a share of the leftover space
		BOX_FILL   = 0x2    // if true, extra space will be given to the child.
		                    // if false, extra space will be added as padding around the child
	};

	Box *PackStart(Widget *child, Uint32 flags = 0);
	Box *PackStart(const WidgetSet &set, Uint32 flags = 0);
	Box *PackEnd(Widget *child, Uint32 flags = 0);
	Box *PackEnd(const WidgetSet &set, Uint32 flags = 0);

	void Remove(Widget *child);
	void Clear();

private:
	BoxOrientation m_orient;
	float m_spacing;

	struct Child {
		Child(Widget *_widget, Uint32 _flags) : widget(_widget), flags(_flags) {}
		Widget   *widget;
		Uint32    flags;
		Point  preferredSize;
		Point  size;
		float     padding;
	};

	std::list<Child> m_children;
	int m_countExpanded;

	Point m_preferredSize;
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
