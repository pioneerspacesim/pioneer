// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Box.h"
#include "Context.h"

namespace UI {

static inline void GetComponentsForOrient(bool horiz, Point::Component &variableComponent, Point::Component &fixedComponent)
{
	if (horiz) {
		variableComponent = Point::X;
		fixedComponent = Point::Y;
	}
	else {
		variableComponent = Point::Y;
		fixedComponent = Point::X;
	}
}

Point Box::PreferredSize()
{
	if (m_children.size() == 0) return Point();

	Point::Component vc, fc;
	GetComponentsForOrient(m_orient == BOX_HORIZONTAL, vc, fc);

	m_preferredSize = Point(0);
	m_minAllocation = 0;
	m_numVariable = 0;

	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		const Point childPreferredSize = (*i).preferredSize = (*i).widget->PreferredSize();

		// they've asked for as much as possible
		if (childPreferredSize[vc] == SIZE_EXPAND) {
			// we'll need to be as big as possible too
			m_preferredSize[vc] = SIZE_EXPAND;

			// count them for later
			m_numVariable++;
		}

        // they asked for a known amount
        else {

            // if we still know our size then we can increase it sanely
		    if (m_preferredSize[vc] != SIZE_EXPAND)
			    // need a bit more
			    m_preferredSize[vc] += childPreferredSize[vc];

			// track minimum known size so we can avoid recounting in Layout()
            m_minAllocation += childPreferredSize[vc];
        }

		// fixed axis should just be as large as our largest
		m_preferredSize[fc] = std::max(m_preferredSize[fc], childPreferredSize[fc]);
	}

	// if there was no variable ones, and thus we're asking for a specific
	// amount of space, add sufficient padding
	if (m_numVariable == 0)
		m_preferredSize[vc] += m_spacing*m_children.size();

	return m_preferredSize;
}

void Box::Layout()
{
	if (m_children.size() == 0) return;

	PreferredSize();

	const Point boxSize = GetSize();

	Point::Component vc, fc;
	GetComponentsForOrient(m_orient == BOX_HORIZONTAL, vc, fc);

	// fast path. we know the exact size that everything wants, so just
	// loop and hand it out
	if (m_numVariable == 0) {
		Point childPos(0), childSize(0);
		for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
			childSize[fc] = boxSize[fc];
			childSize[vc] = (*i).preferredSize[vc];
			SetWidgetDimensions((*i).widget, childPos, childSize);
			childPos[vc] += childSize[vc] + m_spacing;
		}
	}

	// we have one or more children that have requested the maximum size
	// possible. each with a known size gets it, and any remaining space is
	// distributed evenly among the max-sized children. if there is no
	// remaining space, then we're already outside the bounds, so just give
	// them something
	else {
		const size_t sizeAvail = boxSize[vc];
		const size_t sizeMin = sizeAvail/10; // 10%, as good as anything

		const size_t amount = m_minAllocation < sizeAvail ? std::max((sizeAvail-m_minAllocation-m_spacing*(m_children.size()-1))/m_numVariable, sizeMin) : sizeMin;

		Point childPos(0), childSize(0);
		for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
			childSize[fc] = boxSize[fc];
			childSize[vc] = (*i).preferredSize[vc] == SIZE_EXPAND ? amount : (*i).preferredSize[vc];
			SetWidgetDimensions((*i).widget, childPos, childSize);
			childPos[vc] += childSize[vc] + m_spacing;
		}
	}

	LayoutChildren();
}

Box *Box::PackStart(Widget *widget)
{
	assert(widget);
	AddWidget(widget);
	m_children.push_front(Child(widget));
	return this;
}

Box *Box::PackStart(const WidgetSet &set)
{
	for (int i = 0; i < set.numWidgets; ++i)
		PackStart(set.widgets[i]);
	return this;
}

Box *Box::PackEnd(Widget *widget)
{
	assert(widget);
	AddWidget(widget);
	m_children.push_back(Child(widget));
	return this;
}

Box *Box::PackEnd(const WidgetSet &set)
{
	for (int i = 0; i < set.numWidgets; ++i)
		PackEnd(set.widgets[i]);
	return this;
}

void Box::RemoveWidget(Widget *widget)
{
	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i)
		if ((*i).widget == widget) {
			m_children.erase(i);
			Container::RemoveWidget(widget);
			return;
		}
}

void Box::Clear()
{
	m_children.clear();
	Container::RemoveAllWidgets();
}

}
