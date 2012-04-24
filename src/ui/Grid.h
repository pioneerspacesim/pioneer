#ifndef _UI_GRID_H
#define _UI_GRID_H

#include "Container.h"
#include "CellSpec.h"

namespace UI {

class Grid : public Container {
protected:
	friend class Context;
	Grid(Context *context, const CellSpec &rowSpec, const CellSpec &colSpec) : Container(context), m_rowSpec(rowSpec), m_colSpec(colSpec) {}

public:
	virtual vector2f PreferredSize();
	virtual void Layout();

	Grid *SetRow(int rowNum, const WidgetSet &set);
	Grid *SetColumn(int colNum, const WidgetSet &set);

private:
	CellSpec m_rowSpec, m_colSpec;
};

}

#endif
