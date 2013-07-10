// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Table.h"

namespace UI {

Point Table::PreferredSize()
{
	return Point();
}

void Table::Layout()
{
}

Table *Table::SetHeadingRow(const WidgetSet &set)
{
	return this;
}

Table *Table::AddRow(const WidgetSet &set)
{
	return this;
}

}
