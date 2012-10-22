// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_TEXTENTRY_H
#define UI_TEXTENTRY_H

#include "Container.h"
#include "Label.h"

namespace UI {

class TextEntry: public Container {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	TextEntry *SetText(const std::string &text);
	const std::string &GetText() const { return m_label->GetText(); }

	virtual bool IsSelectable() const { return true; }

protected:
	friend class Context;
	TextEntry(Context *context, const std::string &text);

	virtual void HandleKeyPress(const KeyboardEvent &event);

private:
	Label *m_label;
};

}

#endif
