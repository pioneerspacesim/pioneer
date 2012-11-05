// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Expand.h"

namespace UI {

Point Expand::PreferredSize()
{
	if (m_direction == BOTH)
		return Point(SIZE_EXPAND);

	const Point innerPreferredSize(GetInnerWidget() ? GetInnerWidget()->PreferredSize() : Point());

	return m_direction == HORIZONTAL ? Point(SIZE_EXPAND, innerPreferredSize.y) : Point(innerPreferredSize.x, SIZE_EXPAND);
}

}
