// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Table.h"

namespace UI {

class TableLayout {
public:

	void AddRow(std::vector<Widget*> widgets) {
		if (m_columnWidths.size() < widgets.size()) {
			std::size_t i = m_columnWidths.size();
			m_columnWidths.resize(widgets.size());
			for (; i < widgets.size(); i++)
				m_columnWidths[i] = 0;
		}

		int height = 0;
		for (std::size_t i = 0; i < widgets.size(); i++) {
			Widget *w = widgets[i];
			if (!w) continue;
			const Point size(w->CalcLayoutContribution());
			// XXX handle flags
			m_columnWidths[i] = std::max(m_columnWidths[i], size.x);
			height = std::max(height, size.y);
		}

		m_rowHeights.push_back(height);
	}

	const std::vector<int> &ColumnWidths() const {
		return m_columnWidths;
	}

	const std::vector<int> &RowHeights() const {
		return m_rowHeights;
	}

private:
	std::vector<int> m_columnWidths;
	std::vector<int> m_rowHeights;
};


Table::Table(Context *context) : Container(context)
{
}

Point Table::PreferredSize()
{
	return Point();
}

void Table::Layout()
{
	TableLayout layout;
	layout.AddRow(m_heading);
	for (std::vector< std::vector<Widget*> >::const_iterator i = m_rows.begin(); i != m_rows.end(); ++i)
		layout.AddRow(*i);

	Point pos(0);
	const std::vector<int> &colWidths = layout.ColumnWidths();
	const std::vector<int> &rowHeights = layout.RowHeights();

	int row = 0;

	for (std::size_t i = 0; i < m_heading.size(); i++) {
		if (!m_heading[i]) continue;
		Point size(m_heading[i]->CalcLayoutContribution()); // XXX cache contribution in layout?
		SetWidgetDimensions(m_heading[i], pos, size);
		pos.x += colWidths[i];
	}
	pos.y += rowHeights[row++];

	for (std::vector< std::vector<Widget*> >::const_iterator i = m_rows.begin(); i != m_rows.end(); ++i) {
		pos.x = 0;
		for (std::size_t j = 0; j < (*i).size(); j++) {
			if (!(*i)[j]) continue;
			Point size((*i)[j]->CalcLayoutContribution());
			SetWidgetDimensions((*i)[j], pos, size);
			pos.x += colWidths[j];
		}
		pos.y += rowHeights[row++];
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
	m_rows.push_back(set.widgets);

	for (std::size_t i = 0; i < set.widgets.size(); i++) {
		if (!set.widgets[i]) continue;
		AddWidget(set.widgets[i]);
	}

	return this;
}

}
