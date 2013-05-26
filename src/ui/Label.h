// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "Widget.h"
#include "SmartPtr.h"

// single line of text

namespace UI {

class Label: public Widget {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	Label *SetText(const std::string &text);
	const std::string &GetText() const { return m_text; }
	Label *SetColor(float r, float g, float b);

protected:
	friend class Context;
	Label(Context *context, const std::string &text) : Widget(context), m_text(text), m_color(Color::WHITE) {}

private:
	std::string m_text;
	::Color m_color;
	Point m_preferredSize;
};

}

#endif
