// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Panel.h"
#include "gui/GuiScreen.h"

namespace GameUI {

UI::Point Panel::PreferredSize()
{
	return UI::Point(INT_MAX, 80/Gui::Screen::GetCoords2Pixels()[1]);
}

}
