#ifndef _UI_BOX_H
#define _UI_BOX_H

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
	virtual vector2f PreferredSize();
	virtual void Layout();

	struct ChildAttrs {
		ChildAttrs(bool _expand = true, bool _fill = true) : expand(_expand), fill(_fill) {}
		const bool expand; // if true, give this child a share of the leftover space
		const bool fill;   // if true, extra space will be given to the child.
                           // if false, extra space will be added as padding around the child
	};

	Box *PackStart(Widget *child, const ChildAttrs &attrs = ChildAttrs());
	Box *PackStart(const WidgetSet &set, const ChildAttrs &attrs = ChildAttrs());
	Box *PackEnd(Widget *child, const ChildAttrs &attrs = ChildAttrs());
	Box *PackEnd(const WidgetSet &set, const ChildAttrs &attrs = ChildAttrs());
	void Remove(Widget *child);

private:
	BoxOrientation m_orient;
	float m_spacing;

	struct Child {
		Child(Widget *_widget, const ChildAttrs &_attrs) : widget(_widget), attrs(_attrs) {}
		Widget           *widget;
		const ChildAttrs attrs;
		vector2f         preferredSize;
		vector2f         size;
		float            padding;
	};

	std::list<Child> m_children;
	int m_countExpanded;

	vector2f m_preferredSize;
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
