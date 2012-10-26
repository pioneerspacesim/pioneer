// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_GRID_H
#define UI_GRID_H

#include "Container.h"
#include "CellSpec.h"

namespace UI {

class Grid : public Container {
protected:
	friend class Context;
	Grid(Context *context, const CellSpec &rowSpec, const CellSpec &colSpec);

public:
	virtual Point PreferredSize();
	virtual void Layout();

	Grid *SetRow(size_t rowNum, const WidgetSet &set);
	Grid *SetColumn(size_t colNum, const WidgetSet &set);
	Grid *SetCell(size_t colNum, size_t rowNum, Widget *widget);

	void ClearRow(size_t rowNum);
	void ClearColumn(size_t colNum);
	void Clear();

	size_t GetNumRows() const { return m_numRows; }
	size_t GetNumCols() const { return m_numCols; }

protected:
	virtual void RemoveWidget(Widget *);

private:
	CellSpec m_rowSpec, m_colSpec;
	size_t m_numRows, m_numCols;
	std::vector<Widget*> m_widgets;
};

}

#endif
