// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_EXPAND_H
#define UI_EXPAND_H

#include "Single.h"

namespace UI {

class Expand : public Single {
public:
	enum Direction { // <enum scope='UI::Expand' name=UIExpandDirection public>
		BOTH,
		HORIZONTAL,
		VERTICAL,
	};

	virtual Point PreferredSize();

protected:
	friend class Context;
	Expand(Context *context, Direction direction) : Single(context), m_direction(direction) {}

	Direction m_direction;
};

}

#endif
