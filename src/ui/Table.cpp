// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Table.h"

namespace UI {

Table::Table(Context *context) : Container(context),
	m_numColumns(0)
{
}

Point Table::PreferredSize()
{
	return Point();
}

void Table::Layout()
{
	std::size_t numColumns = std::max(m_heading.size(), m_numColumns);
	m_colWidth.resize(numColumns);
	memset(&m_colWidth[0], 0, sizeof(std::size_t)*numColumns); // XXX icky

	for (std::size_t i = 0; i < m_heading.size(); i++) {
		Point size(m_heading[i] ? m_heading[i]->CalcLayoutContribution() : Point(0));
		// XXX handle flags
		m_colWidth[i] = size.x;
	}

	for (std::vector< std::vector<Widget*> >::const_iterator i = m_rows.begin(); i != m_rows.end(); ++i) {
		for (std::size_t j = 0; j < (*i).size(); j++) {
			if (!(*i)[j]) continue;
			Point size((*i)[j] ? (*i)[j]->CalcLayoutContribution() : Point(0));
			m_colWidth[j] = std::max(m_colWidth[j], std::size_t(size.x));
		}
	}

	Point pos(0);

	int height = 0;
	for (std::size_t i = 0; i < m_heading.size(); i++) {
		if (!m_heading[i]) continue;
		Point size(m_heading[i]->CalcLayoutContribution()); // XXX cache contribution
		SetWidgetDimensions(m_heading[i], pos, size);
		pos.x += m_colWidth[i];
		height = std::max(height, size.y);
	}
	pos.y += height;

	for (std::vector< std::vector<Widget*> >::const_iterator i = m_rows.begin(); i != m_rows.end(); ++i) {
		pos.x = 0;
		height = 0;
		for (std::size_t j = 0; j < (*i).size(); j++) {
			if (!(*i)[j]) continue;
			Point size((*i)[j]->CalcLayoutContribution());
			SetWidgetDimensions((*i)[j], pos, size);
			pos.x += m_colWidth[j];
			height = std::max(height, size.y);
		}
		pos.y += height;
	}
}

Table *Table::SetHeadingRow(const WidgetSet &set)
{
	for (std::size_t i = 0; i < m_heading.size(); i++) {
		if (!m_heading[i]) continue;
		RemoveWidget(m_heading[i]);
	}

	m_heading = set.widgets;

	for (std::size_t i = 0; i < m_heading.size(); i++) {
		if (!m_heading[i]) continue;
		AddWidget(m_heading[i]);
	}

	return this;
}

Table *Table::AddRow(const WidgetSet &set)
{
	m_numColumns = std::max(m_numColumns, set.numWidgets);
	m_rows.push_back(set.widgets);

	for (std::size_t i = 0; i < set.widgets.size(); i++) {
		if (!set.widgets[i]) continue;
		AddWidget(set.widgets[i]);
	}

	return this;
}

}
