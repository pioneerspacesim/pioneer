// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

	Grid *SetRow(unsigned int rowNum, const WidgetSet &set);
	Grid *SetColumn(unsigned int colNum, const WidgetSet &set);
	Grid *SetCell(unsigned int colNum, unsigned int rowNum, Widget *widget);

	void ClearRow(unsigned int rowNum);
	void ClearColumn(unsigned int colNum);
	void ClearCell(unsigned int colNum, unsigned int rowNum);
	void Clear();

	unsigned int GetNumRows() const { return m_numRows; }
	unsigned int GetNumCols() const { return m_numCols; }

protected:
	virtual void RemoveWidget(Widget *);

private:
	CellSpec m_rowSpec, m_colSpec;
	unsigned int m_numRows, m_numCols;
	std::vector<Widget*> m_widgets;
};

}

#endif
