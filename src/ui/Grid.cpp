// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Grid.h"

namespace UI {

Grid::Grid(Context *context, const CellSpec &rowSpec, const CellSpec &colSpec) : Container(context),
	m_rowSpec(rowSpec),
	m_colSpec(colSpec),
	m_numRows(colSpec.numCells),
	m_numCols(rowSpec.numCells),
	m_widgets(m_rowSpec.numCells*m_colSpec.numCells)
{
	Clear();
}

Point Grid::PreferredSize()
{
	Point preferredSize;

	for (unsigned int rowNum = 0; rowNum < m_numRows; rowNum++) {
		Point rowSize;

		for (unsigned int colNum = 0; colNum < m_numCols; colNum++) {
			const unsigned int n = rowNum*m_numCols+colNum;
			Widget *w = m_widgets[n];
			if (!w) continue;

			const Point childPreferredSize = w->CalcLayoutContribution();
			rowSize.x = SizeAdd(childPreferredSize.x, rowSize.x);
			rowSize.y = std::max(childPreferredSize.y, rowSize.y);
		}

		preferredSize.x = std::max(preferredSize.x, rowSize.x);
		preferredSize.y = SizeAdd(preferredSize.y, rowSize.y);
	}

	return preferredSize;
}

void Grid::Layout()
{
	const Point size = GetSize();

	Point childPos, childSize;
	for (unsigned int rowNum = 0; rowNum < m_numRows; rowNum++) {
		childSize.y = m_colSpec.cellPercent[rowNum]*size.y;

		childPos.x = 0;
		for (unsigned int colNum = 0; colNum < m_numCols; colNum++) {
			childSize.x = m_rowSpec.cellPercent[colNum]*size.x;

			const unsigned int n = rowNum*m_numCols+colNum;
			if (m_widgets[n])
				SetWidgetDimensions(m_widgets[n], childPos, m_widgets[n]->CalcSize(childSize));

			childPos.x += childSize.x;
		}

		childPos.y += childSize.y;
	}

	LayoutChildren();
}

Grid *Grid::SetRow(unsigned int rowNum, const WidgetSet &set)
{
	assert(set.numWidgets <= m_rowSpec.numCells);
	assert(rowNum >= 0 && rowNum < m_numRows);

	for (unsigned int i = 0; i < set.numWidgets; i++) {
		const unsigned int n = rowNum*m_numCols+i;
		if (m_widgets[n])
			RemoveWidget(m_widgets[n]);
		m_widgets[n] = set.widgets[i];
		AddWidget(m_widgets[n]);
	}

	return this;
}

Grid *Grid::SetColumn(unsigned int colNum, const WidgetSet &set)
{
	assert(set.numWidgets <= m_colSpec.numCells);
	assert(colNum >= 0 && colNum < m_numCols);

	for (unsigned int i = 0; i < set.numWidgets; i++) {
		const unsigned int n = i*m_numCols+colNum;
		if (m_widgets[n])
			RemoveWidget(m_widgets[n]);
		m_widgets[n] = set.widgets[i];
		AddWidget(m_widgets[n]);
	}

	return this;
}

Grid *Grid::SetCell(unsigned int colNum, unsigned int rowNum, Widget *widget)
{
	assert(colNum >= 0 && colNum < m_numCols);
	assert(rowNum >= 0 && rowNum < m_numRows);

	const unsigned int n = rowNum*m_numCols+colNum;
	if (m_widgets[n])
		RemoveWidget(m_widgets[n]);
	m_widgets[n] = widget;
	AddWidget(m_widgets[n]);

	return this;
}

void Grid::ClearRow(unsigned int rowNum)
{
	assert(rowNum >= 0 && rowNum < m_numRows);

	for (unsigned int i = 0; i < m_numCols; i++) {
		const unsigned int n = rowNum*m_numCols+i;
		if (m_widgets[n]) {
			Container::RemoveWidget(m_widgets[n]);
			m_widgets[n] = 0;
		}
	}
}

void Grid::ClearColumn(unsigned int colNum)
{
	assert(colNum >= 0 && colNum < m_numRows);

	for (unsigned int i = 0; i < m_numRows; i++) {
		const unsigned int n = i*m_numCols+colNum;
		if (m_widgets[n]) {
			Container::RemoveWidget(m_widgets[n]);
			m_widgets[n] = 0;
		}
	}
}

void Grid::ClearCell(unsigned int colNum, unsigned int rowNum)
{
	assert(colNum >= 0 && colNum < m_numCols);
	assert(rowNum >= 0 && rowNum < m_numRows);

	const unsigned int n = rowNum*m_numCols+colNum;
	if (m_widgets[n])
		RemoveWidget(m_widgets[n]);
	m_widgets[n] = 0;
}

void Grid::Clear()
{
	for (unsigned int i = 0; i < m_numRows*m_numCols; i++)
		m_widgets[i] = 0;

	Container::RemoveAllWidgets();
}

void Grid::RemoveWidget(Widget *widget)
{
	for (unsigned int i = 0; i < m_numRows*m_numCols; i++)
		if (m_widgets[i] == widget) {
			Container::RemoveWidget(widget);
			m_widgets[i] = 0;
		}
}

}
