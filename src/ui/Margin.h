// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_MARGIN_H
#define UI_MARGIN_H

#include "Single.h"

namespace UI {

class Margin : public Single {
public:
	virtual Point PreferredSize();
	virtual void Layout();

protected:
    friend class Context;
	Margin(Context *context, float margin) : Single(context), m_margin(margin) {}

private:
	float m_margin;
};

}

#endif
