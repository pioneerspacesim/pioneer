// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_BUTTON_H
#define UI_BUTTON_H

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
