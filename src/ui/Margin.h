#ifndef _UI_MARGIN_H
#define _UI_MARGIN_H

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
