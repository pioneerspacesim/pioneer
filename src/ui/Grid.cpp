#include "Grid.h"

namespace UI {

void Grid::Layout()
{
}

Grid *Grid::SetRow(int rowNum, const WidgetSet &set)
{
	assert(set.numWidgets == m_rowSpec.numCells);
	assert(rowNum >= 0 && rowNum < m_colSpec.numCells);

	for (int i = 0; i < set.numWidgets; i++) {
		const int n = rowNum*m_colSpec.numCells+i;
		if (m_widgets[n]) delete m_widgets[n]; // XXX widget deletion
		m_widgets[n] = set.widgets[i];
	}

	return this;
}

Grid *Grid::SetColumn(int colNum, const WidgetSet &set)
{
	assert(set.numWidgets == m_colSpec.numCells);
	assert(colNum >= 0 && colNum < m_rowSpec.numCells);

	for (int i = 0; i < set.numWidgets; i++) {
		const int n = i*m_colSpec.numCells+colNum;
		if (m_widgets[n]) delete m_widgets[n]; // XXX widget deletion
		m_widgets[n] = set.widgets[i];
	}

	return this;
}

Grid *Grid::SetCell(int colNum, int rowNum, Widget *widget)
{
	assert(colNum > 0 && colNum < m_rowSpec.numCells);
	assert(rowNum > 0 && rowNum < m_colSpec.numCells);

	const int n = rowNum*m_colSpec.numCells+colNum;
	if (m_widgets[n]) delete m_widgets[n]; // XXX widget deletion
	m_widgets[n] = widget;

	return this;
}

}
