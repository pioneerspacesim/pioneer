// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Table.h"
#include "Context.h"
#include "Slider.h"

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
	m_rowSpacing(0),
	m_columnSpacing(0),
	m_dirty(false)
{
}

Point Table::Inner::PreferredSize()
{
	m_preferredSizes.clear();
	m_preferredSize = Point();

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

		if (!row.empty() && m_columnSpacing)
			m_preferredSize.x += (row.size()-1)*m_columnSpacing;

		m_preferredSize.x = std::max(m_preferredSize.x, rowSize.x);
		m_preferredSize.y += rowSize.y;
	}

	if (!m_rows.empty() && m_rowSpacing)
		m_preferredSize.y += (m_rows.size()-1)*m_rowSpacing;

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
			pos.x += colWidths[j] + m_columnSpacing;
			height = std::max(height, preferredSize.y);
		}
		pos.y += height + m_rowSpacing;
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

void Table::Inner::SetRowSpacing(int spacing)
{
	m_rowSpacing = spacing;
	m_dirty = true;
}

void Table::Inner::SetColumnSpacing(int spacing)
{
	m_columnSpacing = spacing;
	m_dirty = true;
}

void Table::Inner::SetSpacing(int spacing)
{
	m_rowSpacing = m_columnSpacing = spacing;
	m_dirty = true;
}

Table::Table(Context *context) : Container(context),
	m_dirty(false)
{
	m_header.Reset(new Table::Inner(GetContext(), m_layout));
	AddWidget(m_header.Get());

	m_body.Reset(new Table::Inner(GetContext(), m_layout));
	AddWidget(m_body.Get());

	m_slider.Reset(GetContext()->VSlider());
	m_slider->onValueChanged.connect(sigc::mem_fun(this, &Table::OnScroll));
}

Point Table::PreferredSize()
{
	m_layout.Clear();

	m_header->AccumulateLayout();
	m_body->AccumulateLayout();

	m_dirty = false;

	const Point layoutSize = m_layout.GetSize();
	const Point sliderSize = m_slider->PreferredSize();

	const Point headerPreferredSize = m_header->PreferredSize();
	const Point bodyPreferredSize = m_body->PreferredSize();

	return Point(layoutSize.x+sliderSize.x, headerPreferredSize.y+bodyPreferredSize.y);
}

void Table::Layout()
{
	if (m_dirty)
		PreferredSize();

	Point size = GetSize();

	Point preferredSize(m_header->PreferredSize());
	SetWidgetDimensions(m_header.Get(), Point(), Point(size.x, preferredSize.y));
	int top = preferredSize.y;
	size.y -= top;

	Point sliderSize;

	preferredSize = m_body->PreferredSize();
	if (preferredSize.y <= size.y) {
		if (m_slider->GetContainer()) {
			m_onMouseWheelConn.disconnect();
			RemoveWidget(m_slider.Get());
		}
	}
	else {
		AddWidget(m_slider.Get());
		m_onMouseWheelConn = onMouseWheel.connect(sigc::mem_fun(this, &Table::OnMouseWheel));
		sliderSize = m_slider->PreferredSize();
		SetWidgetDimensions(m_slider.Get(), Point(size.x-sliderSize.x, top), Point(sliderSize.x, size.y));
		size.x -= sliderSize.x;
	}

	SetWidgetDimensions(m_body.Get(), Point(0, top), size);

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

Table *Table::SetRowSpacing(int spacing)
{
	m_header->SetRowSpacing(spacing);
	m_body->SetRowSpacing(spacing);
	m_dirty = true;
	return this;
}

Table *Table::SetColumnSpacing(int spacing)
{
	m_header->SetColumnSpacing(spacing);
	m_body->SetColumnSpacing(spacing);
	m_dirty = true;
	return this;
}

Table *Table::SetSpacing(int spacing)
{
	m_header->SetSpacing(spacing);
	m_body->SetSpacing(spacing);
	m_dirty = true;
	return this;
}


Table *Table::SetHeadingFont(Font font)
{
	m_header->SetFont(font);
	return this;
}


void Table::OnScroll(float value)
{
	m_body->SetDrawOffset(Point(0, -float(m_body->PreferredSize().y-(GetSize().y-m_header->PreferredSize().y))*value));
}

bool Table::OnMouseWheel(const MouseWheelEvent &event)
{
	m_slider->SetValue(m_slider->GetValue() + (event.direction == MouseWheelEvent::WHEEL_UP ? -0.01f : 0.01f));
	return true;
}


}
