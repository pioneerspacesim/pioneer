#ifndef _UI_ALIGN_H
#define _UI_ALIGN_H

#include "Single.h"

namespace UI {

class Align : public Single {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	enum Direction { // <enum scope='UI::Align' name=UIAlignDirection>
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
