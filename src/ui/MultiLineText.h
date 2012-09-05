#ifndef _UI_MULTILINETEXT_H
#define _UI_MULTILINETEXT_H

#include "Widget.h"
#include "SmartPtr.h"

namespace UI {

class TextLayout;

class MultiLineText: public Widget {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();
	virtual void Draw();

	MultiLineText *SetText(const std::string &text);
	MultiLineText *AppendText(const std::string &text);

protected:
	friend class Context;
	MultiLineText(Context *context, const std::string &text);

private:
	std::string m_text;
	ScopedPtr<TextLayout> m_layout;
	vector2f m_preferredSize;
};

}

#endif
