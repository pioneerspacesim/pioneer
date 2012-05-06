#include "Box.h"

namespace UI {

static inline void GetComponentsForOrient(bool horiz, vector2f::Component &variableComponent, vector2f::Component &fixedComponent)
{
	if (horiz) {
		variableComponent = vector2f::X;
		fixedComponent = vector2f::Y;
	}
	else {
		variableComponent = vector2f::Y;
		fixedComponent = vector2f::X;
	}
}

vector2f Box::PreferredSize()
{
	if (m_children.size() == 0) return 0;

	vector2f::Component vc, fc;
	GetComponentsForOrient(m_orient == BOX_HORIZONTAL, vc, fc);
	
	m_preferredSize = vector2f(0);

	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		const vector2f childPreferredSize = (*i).preferredSize = (*i).widget->PreferredSize();

		m_preferredSize[vc] = std::min(m_preferredSize[vc]+childPreferredSize[vc], FLT_MAX);
		m_preferredSize[fc] = std::max(m_preferredSize[fc], childPreferredSize[fc]);
	}

	if (m_children.size() > 1)
		m_preferredSize[vc] += m_spacing * (m_children.size()-1);

	return m_preferredSize;
}

void Box::Layout()
{
	if (m_children.size() == 0) return;

	PreferredSize();

	const vector2f boxSize = GetSize();

	vector2f::Component vc, fc;
	GetComponentsForOrient(m_orient == BOX_HORIZONTAL, vc, fc);

	float sizeRemaining = boxSize[vc] - (m_spacing * (m_children.size()-1));

	vector2f childPos(0);

	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		(*i).padding = 0;

		float childSize = 0;

		if (boxSize[vc] >= m_preferredSize[vc])
			childSize = (*i).preferredSize[vc];
		else
			childSize = boxSize[vc]/m_children.size();

		(*i).size[vc] = childSize;
		(*i).size[fc] = boxSize[fc];

		sizeRemaining -= childSize;

		if (m_countExpanded == 0) {
			SetWidgetDimensions((*i).widget, childPos, (*i).size);
			childPos[vc] += childSize + m_spacing;
		}
	}

	if (m_countExpanded > 0) {
		int candidates = m_countExpanded;

		while (candidates > 0 && sizeRemaining > 0) {
			float allocation = sizeRemaining / candidates;

			for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
				if (!(*i).attrs.expand) continue;

				float amountAdded;
				if (!(*i).attrs.fill) {
					(*i).padding += allocation * 0.5;
					amountAdded = allocation;
				}
				else {
					(*i).size[vc] += allocation;
					amountAdded = allocation;
				}

				sizeRemaining -= amountAdded;
			}
		}

		for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
			vector2f pos = childPos;
			pos[vc] += (*i).padding;

			SetWidgetDimensions((*i).widget, pos, (*i).size);

			childPos[vc] = pos[vc] + (*i).size[vc] + (*i).padding + m_spacing;
		}
	}

	LayoutChildren();
}

void Box::RequestResize()
{
	if (GetContainer()) GetContainer()->RequestResize();
	Container::RequestResize();
}

Box *Box::PackStart(Widget *widget, const ChildAttrs &attrs)
{
	AddWidget(widget);
	m_children.push_front(Child(widget, attrs));
	if (attrs.expand) m_countExpanded++;
	if (GetContainer()) GetContainer()->RequestResize();
	return this;
}

Box *Box::PackStart(const WidgetSet &set, const ChildAttrs &attrs)
{
	for (int i = 0; i < set.numWidgets; ++i)
		PackStart(set.widgets[i], attrs);
	if (GetContainer()) GetContainer()->RequestResize();
	return this;
}

Box *Box::PackEnd(Widget *widget, const ChildAttrs &attrs)
{
	AddWidget(widget);
	m_children.push_back(Child(widget, attrs));
	if (attrs.expand) m_countExpanded++;
	if (GetContainer()) GetContainer()->RequestResize();
	return this;
}

Box *Box::PackEnd(const WidgetSet &set, const ChildAttrs &attrs)
{
	for (int i = 0; i < set.numWidgets; ++i)
		PackEnd(set.widgets[i], attrs);
	if (GetContainer()) GetContainer()->RequestResize();
	return this;
}

void Box::Remove(Widget *widget)
{
	for (std::list<Child>::iterator i = m_children.begin(); i != m_children.end(); ++i)
		if ((*i).widget == widget) {
			m_children.erase(i);
			RemoveWidget(widget);
			if (GetContainer()) GetContainer()->RequestResize();
			return;
		}
}

void Box::Clear()
{
	m_children.clear();
	Container::RemoveAllWidgets();
	if (GetContainer()) GetContainer()->RequestResize();
}

}
