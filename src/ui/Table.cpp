// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Table.h"

namespace UI {

void Table::LayoutAccumulator::AddRow(const std::vector<Widget*> &widgets)
{
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
	m_size.y += height;

	m_size.x = 0;
	for (std::size_t i = 0; i < m_columnWidths.size(); i++)
		m_size.x += m_columnWidths[i];
}

void Table::LayoutAccumulator::Clear()
{
	m_columnWidths.clear();
	m_size = Point();
}

Table::Inner::Inner(Context *context, LayoutAccumulator &layout) : Container(context),
	m_layout(layout),
	m_dirty(false)
{
}

Point Table::Inner::PreferredSize()
{
	m_preferredSizes.clear();

	for (std::size_t i = 0; i < m_rows.size(); i++) {
		const std::vector<Widget*> &row = m_rows[i];
		Point rowSize;
		std::vector<Point> preferredSizes(row.size());
		for (std::size_t j = 0; j < row.size(); j++) {
			Widget *w = row[j];
			if (!w) continue;
			Point size(w->CalcLayoutContribution());
			preferredSizes[j] = size;
			rowSize.x += size.x;
			rowSize.y = std::max(rowSize.y, size.y);
		}
		m_preferredSizes.push_back(preferredSizes);

		m_preferredSize.x = std::max(m_preferredSize.x, rowSize.x);
		m_preferredSize.y += rowSize.y;
	}

	m_dirty = false;

	return m_preferredSize;
}

void Table::Inner::Layout()
{
	if (m_dirty)
		PreferredSize();

	Point pos;
	const std::vector<int> &colWidths = m_layout.ColumnWidths();

	for (std::size_t i = 0; i < m_rows.size(); i++) {
		const std::vector<Widget*> &row = m_rows[i];
		const std::vector<Point> &preferredSizes = m_preferredSizes[i];
		pos.x = 0;
		int height = 0;
		for (std::size_t j = 0; j < row.size(); j++) {
			Widget *w = row[j];
			const Point &preferredSize = preferredSizes[j];
			if (!w) continue;
			SetWidgetDimensions(w, pos, preferredSize);
			pos.x += colWidths[j];
			height = std::max(height, preferredSize.y);
		}
		pos.y += height;
	}

	LayoutChildren();
}

void Table::Inner::AddRow(const std::vector<Widget*> &widgets)
{
	m_rows.push_back(widgets);

	Point rowSize;
	for (std::size_t i = 0; i < widgets.size(); i++) {
		if (!widgets[i]) continue;
		AddWidget(widgets[i]);
	}

	m_dirty = true;
}

void Table::Inner::Clear()
{
	for (std::vector< std::vector<Widget*> >::const_iterator i = m_rows.begin(); i != m_rows.end(); ++i) {
		for (std::size_t j = 0; j < (*i).size(); j++) {
			if (!(*i)[j]) continue;
			RemoveWidget((*i)[j]);
		}
	}

	m_rows.clear();
	m_preferredSize = Point();
	m_dirty = false;
}

void Table::Inner::AccumulateLayout()
{
	for (std::vector< std::vector<Widget*> >::const_iterator i = m_rows.begin(); i != m_rows.end(); ++i)
		m_layout.AddRow(*i);
}

Table::Table(Context *context) : Container(context),
	m_dirty(false)
{
	m_header.Reset(new Table::Inner(GetContext(), m_layout));
	AddWidget(m_header.Get());

	m_body.Reset(new Table::Inner(GetContext(), m_layout));
	AddWidget(m_body.Get());

}

Point Table::PreferredSize()
{
	m_layout.Clear();

	m_header->AccumulateLayout();
	m_body->AccumulateLayout();

	m_dirty = false;

	return m_layout.GetSize();
}

void Table::Layout()
{
	if (m_dirty)
		PreferredSize();

	const Point &layoutSize = m_layout.GetSize();

	Point pos;

	{
		Point preferredSize(m_header->PreferredSize());
		SetWidgetDimensions(m_header.Get(), pos, Point(layoutSize.x, preferredSize.y));
		pos.y = preferredSize.y;
	}

	{
		Point preferredSize(m_body->PreferredSize());
		SetWidgetDimensions(m_body.Get(), pos, Point(layoutSize.x, preferredSize.y));
	}

	LayoutChildren();
}

Table *Table::SetHeadingRow(const WidgetSet &set)
{
	m_header->Clear();
	m_header->AddRow(set.widgets);

	m_dirty = true;
	return this;
}

Table *Table::AddRow(const WidgetSet &set)
{
	m_body->AddRow(set.widgets);

	m_dirty = true;

	return this;

}

}
