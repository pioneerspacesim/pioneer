// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
	if (m_children.empty()) return Point();

	Point::Component vc, fc;
	GetComponentsForOrient(m_orient == BOX_HORIZONTAL, vc, fc);

	m_preferredSize = Point(0);
	m_numVariable = 0;

	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		const Point contribSize = (*i).contribSize = (*i).widget->CalcLayoutContribution();

		// if they're not contributing anything on the fixed axis then we need
		// to defer their inclusion until we know the size of the fixed axis
		// and can ask for their best size. otherwise we can assign the wrong
		// amount of space on the variable axis if they would get aspect
		// scaled down
		if (contribSize[fc] == 0)
			continue;

		// they've asked for as much as possible
		if (contribSize[vc] == SIZE_EXPAND) {
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
			    m_preferredSize[vc] += contribSize[vc];
		}

		// fixed axis should just be as large as our largest
		m_preferredSize[fc] = std::max(m_preferredSize[fc], contribSize[fc]);
	}

	// we have fixed size, so we can do the deferred ones now
	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		// already handled
		if ((*i).contribSize[fc] != 0)
			continue;

		// calculate final size usng its own variable size and the accumulated
		// fixed size
		Point availableSize;
		availableSize[vc] = (*i).contribSize[vc];
		availableSize[fc] = m_preferredSize[fc];
		const Point contribSize = (*i).contribSize = (*i).widget->CalcSize(availableSize);

		// repeat of above, sigh
		if (contribSize[vc] == SIZE_EXPAND) {
			m_preferredSize[vc] = SIZE_EXPAND;
			m_numVariable++;
		}
		else {
			if (m_preferredSize[vc] != SIZE_EXPAND)
			    m_preferredSize[vc] += contribSize[vc];
		}
		m_preferredSize[fc] = std::max(m_preferredSize[fc], contribSize[fc]);
	}

	// if there was no variable ones, and thus we're asking for a specific
	// amount of space, add sufficient padding
	if (m_numVariable == 0)
		m_preferredSize[vc] += m_spacing*m_children.size();

	return m_preferredSize;
}

void Box::Layout()
{
	if (m_children.empty()) return;

	PreferredSize();

	const Point& boxSize = GetSize();

	Point::Component vc, fc;
	GetComponentsForOrient(m_orient == BOX_HORIZONTAL, vc, fc);

	// fast path. we know the exact size that everything wants, so just
	// loop and hand it out
	if (m_numVariable == 0) {

		// we got what we asked for so everyone can have what they want
		if (boxSize[vc] >= m_preferredSize[vc]) {
			for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
				(*i).size[fc] = boxSize[fc];
				(*i).size[vc] = (*i).contribSize[vc];
			}
		}

		// didn't get enough, so we have share it around
		else {
			// we can certainly afford to give everyone this much
			int availSize = boxSize[vc] - (m_spacing * (m_children.size()-1));
			int minSize = availSize / m_children.size();
			int remaining = availSize;
			int wantMore = 0;

			// loop and hand it out
			for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
				(*i).size[fc] = boxSize[fc];
				(*i).size[vc] = std::min(minSize, (*i).contribSize[vc]);
				remaining -= (*i).size[vc];

				// if this one didn't get us much as it wanted then count it
				// if we have any left over we can give it some more
				if ((*i).size[vc] < (*i).contribSize[vc])
					wantMore++;
			}

			// if there's some remaining and not everyone got what they wanted, hand it out
			assert(remaining >= 0);
			if (remaining && wantMore) {
				int extra = remaining / wantMore;
				for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
					if ((*i).size[vc] < (*i).contribSize[vc])
						(*i).size[vc] += extra;
				}
			}
		}
	}

	// we have one or more children that have requested the maximum size possible
	else {

		int availSize = boxSize[vc] - (m_spacing * (m_children.size()-1));
		int remaining = availSize;

		// fixed ones first
		if (m_children.size() > m_numVariable) {
			// distribute evenly among the fixed ones
			int minSize = availSize / (m_children.size() - m_numVariable);

			// loop and hand it out
			for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
				// don't give any to expanding ones yet
				if ((*i).contribSize[vc] == SIZE_EXPAND)
					continue;

				(*i).size[fc] = boxSize[fc];
				(*i).size[vc] = std::min(minSize, (*i).contribSize[vc]);
				remaining -= (*i).size[vc];
			}
		}

		// if there's any space remaining, distribute it among the expanding widgets
		assert(remaining >= 0);
		if (remaining) {
			int extra = remaining / m_numVariable;
			for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
				if ((*i).contribSize[vc] != SIZE_EXPAND)
					continue;

				(*i).size[fc] = boxSize[fc];
				(*i).size[vc] = extra;
			}
		}
	}

	// now loop over them again and position
	Point childPos(0);
	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		const Point actualSize((*i).widget->CalcSize((*i).size));
		SetWidgetDimensions((*i).widget, childPos, actualSize);
		childPos[vc] += actualSize[vc] + m_spacing;
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
	for (size_t i = 0; i < set.numWidgets; ++i)
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
	for (size_t i = 0; i < set.numWidgets; ++i)
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
