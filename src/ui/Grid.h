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

	Grid *SetRow(int rowNum, const WidgetSet &set);
	Grid *SetColumn(int colNum, const WidgetSet &set);
	Grid *SetCell(int colNum, int rowNum, Widget *widget);

	void ClearRow(int rowNum);
	void ClearColumn(int colNum);
	void Clear();

	size_t GetNumRows() const { return m_numRows; }
	size_t GetNumCols() const { return m_numCols; }

private:
	CellSpec m_rowSpec, m_colSpec;
	int m_numRows, m_numCols;
	std::vector<Widget*> m_widgets;
};

}

#endif
