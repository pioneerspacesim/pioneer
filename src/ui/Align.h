// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_ALIGN_H
#define UI_ALIGN_H

#include "Single.h"

namespace UI {

class Align : public Single {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	enum Direction { // <enum scope='UI::Align' name=UIAlignDirection public>
		TOP_LEFT,
		TOP,
		TOP_RIGHT,
		LEFT,
		MIDDLE,
		RIGHT,
		BOTTOM_LEFT,
		BOTTOM,
		BOTTOM_RIGHT
	};

protected:
    friend class Context;
	Align(Context *context, Direction direction) : Single(context), m_direction(direction) {}

private:
	Direction m_direction;
};

}

#endif
