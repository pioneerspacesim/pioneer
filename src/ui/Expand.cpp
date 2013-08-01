// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Expand.h"

namespace UI {

Point Expand::PreferredSize()
{
	const Point innerPreferredSize(GetInnerWidget() ? GetInnerWidget()->CalcLayoutContribution() : Point());

	switch (m_direction) {
		case BOTH:
			SetSizeControlFlags(EXPAND_WIDTH | EXPAND_HEIGHT);
			break;
		case HORIZONTAL:
			SetSizeControlFlags(EXPAND_WIDTH);
			break;
		case VERTICAL:
			SetSizeControlFlags(EXPAND_HEIGHT);
			break;
	}

	return innerPreferredSize;
}

}
