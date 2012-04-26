#ifndef _UI_DROPDOWN_H
#define _UI_DROPDOWN_H

#include "Widget.h"

namespace UI {

class Context;
class Background;

class DropDown : public Widget {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();
	virtual void Draw();

	DropDown *AddOption(const std::string &text);

protected:
	friend class Context;
	DropDown(Context *context);

private:
	Background *m_mainContainer;
	Background *m_optionList;
};

}

#endif
