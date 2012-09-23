#ifndef _UI_BUTTON_H
#define _UI_BUTTON_H

#include "Single.h"

namespace UI {

class Button: public Single {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

protected:
	friend class Context;
	Button(Context *context): Single(context) {}
};

}

#endif
