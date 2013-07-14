// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_TABLE_H
#define UI_TABLE_H

#include "Container.h"

namespace UI {

class Table: public Container {
protected:
	friend class Context;
	Table(Context *context);

public:
	virtual Point PreferredSize();
	virtual void Layout();

	Table *SetHeadingRow(const WidgetSet &set);
	Table *AddRow(const WidgetSet &set);

private:
	std::vector<Widget*> m_heading;
	std::vector< std::vector<Widget*> > m_rows;

	std::size_t m_numColumns;
	std::vector<std::size_t> m_colWidth;
};

}

#endif

