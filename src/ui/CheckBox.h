#ifndef _UI_CHECKBOX_H
#define _UI_CHECKBOX_H

#include "Widget.h"

namespace UI {

class CheckBox: public Widget {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

protected:
	friend class Context;
	CheckBox(Context *context): Widget(context), m_checked(false) {}

	void HandleClick();

private:
	bool m_checked;
};

}

#endif
