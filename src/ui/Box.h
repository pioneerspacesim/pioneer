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

	Box(Context *context, BoxOrientation orient);

public:
	virtual Metrics GetMetrics(const vector2f &hint);
	virtual void Layout();

	struct ChildAttrs {
		ChildAttrs(bool _expand = true, bool _fill = true) : expand(_expand), fill(_fill) {}
		bool  expand;
		bool  fill;
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
		Metrics          metrics;
		vector2f         size;
		float            padding;
	};

	std::list<Child> m_children;
	int m_countExpanded;

	void CalculateMetrics(const vector2f &hint);
	Metrics m_metrics;
	bool m_needMetricsRecalc;
};

class VBox: public Box {
protected:
	friend class Context;
	VBox(Context *context): Box(context, BOX_VERTICAL) {}
};
	
class HBox: public Box {
protected:
	friend class Context;
	HBox(Context *context): Box(context, BOX_HORIZONTAL) {}
};

}

#endif
