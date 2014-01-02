// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_PANEL_H
#define GAMEUI_PANEL_H

// XXX one day this will implement the panel. today its just a spacer to help
// the layout while we transition from the old gui

#include "ui/Context.h"

namespace GameUI {

class Panel : public UI::Widget {
public:
	Panel(UI::Context *context) : UI::Widget(context) {}

	virtual UI::Point PreferredSize();
	virtual void Draw() {}
};

}

#endif
