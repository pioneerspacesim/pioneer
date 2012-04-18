#ifndef _UI_LABEL_H
#define _UI_LABEL_H

#include "Widget.h"
#include "SmartPtr.h"

namespace UI {

class TextLayout;

// XXX make into a generic text widget with layout options (direction,
// multiline, etc). make Label subclass that
class Label: public Widget {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();
	virtual void Draw();

protected:
	friend class Context;
	Label(Context *context, const std::string &text);

private:
	std::string m_text;
	ScopedPtr<TextLayout> m_layout;
};

}

#endif
