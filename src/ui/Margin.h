// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_MARGIN_H
#define UI_MARGIN_H

#include "Single.h"

namespace UI {

class Margin : public Single {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	enum Direction { // <enum scope='UI::Margin' name=UIMarginDirection public>
		ALL,
		HORIZONTAL,
		VERTICAL,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM
	};

protected:
	friend class Context;
	Margin(Context *context, int margin, Direction direction) : Single(context), m_margin(margin), m_direction(direction) {}

private:
	int m_margin;
	Direction m_direction;
};

}

#endif
