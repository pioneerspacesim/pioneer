// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_SELECTORBOX_H
#define UI_SELECTORBOX_H

#include "Single.h"

namespace UI {

class SelectorBox: public Single {
public:
	enum Shape { // <enum scope='UI::SelectorBox' name=UISelectorShape public>
		RECT,
		BRACKET,
	};

	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	SelectorBox *SetColor(const Color &c) { m_color = c; return this; }
	SelectorBox *SetShown(bool shown) { m_shown = shown; return this; }

	bool IsShown() const { return m_shown; }

protected:
	friend class Context;
	SelectorBox(Context *context, Shape shape):
		Single(context), m_color(255,255,255,255), m_shape(shape), m_shown(true) {}

private:
	Color m_color;
	Shape m_shape;
	bool m_shown;
};

}

#endif
