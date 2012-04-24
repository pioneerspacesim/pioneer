#ifndef _UI_GRID_H
#define _UI_GRID_H

#include "Container.h"
#include "CellSpec.h"

namespace UI {

class Grid : public Container {
protected:
	friend class Context;
	Grid(Context *context, const CellSpec &rowSpec, const CellSpec &colSpec) : Container(context), m_rowSpec(rowSpec), m_colSpec(colSpec), m_widgets(m_rowSpec.numCells*m_colSpec.numCells) {}

public:
	virtual vector2f PreferredSize() { return 0; }
	virtual void Layout();

	Grid *SetRow(int rowNum, const WidgetSet &set);
	Grid *SetColumn(int colNum, const WidgetSet &set);
	Grid *SetCell(int colNum, int rowNum, Widget *widget);

private:
	CellSpec m_rowSpec, m_colSpec;
	std::vector<Widget*> m_widgets;
};

}

#endif
