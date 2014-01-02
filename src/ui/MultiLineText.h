// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_MULTILINETEXT_H
#define UI_MULTILINETEXT_H

#include "Widget.h"
#include "SmartPtr.h"

namespace UI {

class TextLayout;

class MultiLineText: public Widget {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	virtual Widget *SetFont(Font font);

	MultiLineText *SetText(const std::string &text);
	MultiLineText *AppendText(const std::string &text);

protected:
	friend class Context;
	MultiLineText(Context *context, const std::string &text);

private:
	std::string m_text;
	std::unique_ptr<TextLayout> m_layout;
	Point m_preferredSize;
};

}

#endif
