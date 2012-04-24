#include "Grid.h"

namespace UI {

void Grid::Layout()
{
	const vector2f size = GetSize();

	vector2f childPos, childSize;
	for (int rowNum = 0; rowNum < m_colSpec.numCells; rowNum++) {
		childSize.y = m_colSpec.cellPercent[rowNum]*size.y;

		childPos.x = 0;
		for (int colNum = 0; colNum < m_rowSpec.numCells; colNum++) {
			childSize.x = m_rowSpec.cellPercent[colNum]*size.x;

			const int n = rowNum*m_colSpec.numCells+colNum;
			if (m_widgets[n])
				SetWidgetDimensions(m_widgets[n], childPos, childSize);

			childPos.x += childSize.x;
		}

		childPos.y += childSize.y;
	}

	LayoutChildren();
}

Grid *Grid::SetRow(int rowNum, const WidgetSet &set)
{
	assert(set.numWidgets <= m_rowSpec.numCells);
	assert(rowNum >= 0 && rowNum < m_colSpec.numCells);

	for (int i = 0; i < set.numWidgets; i++) {
		const int n = rowNum*m_colSpec.numCells+i;
		if (m_widgets[n]) {
			RemoveWidget(m_widgets[n]);
			delete m_widgets[n]; // XXX widget deletion
		}
		m_widgets[n] = set.widgets[i];
		AddWidget(m_widgets[n]);
	}

	return this;
}

Grid *Grid::SetColumn(int colNum, const WidgetSet &set)
{
	assert(set.numWidgets <= m_colSpec.numCells);
	assert(colNum >= 0 && colNum < m_rowSpec.numCells);

	for (int i = 0; i < set.numWidgets; i++) {
		const int n = i*m_colSpec.numCells+colNum;
		if (m_widgets[n]) {
			RemoveWidget(m_widgets[n]);
			delete m_widgets[n]; // XXX widget deletion
		}
		m_widgets[n] = set.widgets[i];
		AddWidget(m_widgets[n]);
	}

	return this;
}

Grid *Grid::SetCell(int colNum, int rowNum, Widget *widget)
{
	assert(colNum > 0 && colNum < m_rowSpec.numCells);
	assert(rowNum > 0 && rowNum < m_colSpec.numCells);

	const int n = rowNum*m_colSpec.numCells+colNum;
	if (m_widgets[n]) {
		RemoveWidget(m_widgets[n]);
		delete m_widgets[n]; // XXX widget deletion
	}
	m_widgets[n] = widget;
	AddWidget(m_widgets[n]);

	return this;
}

}
