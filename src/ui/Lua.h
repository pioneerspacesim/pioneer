// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_LUA_H
#define UI_LUA_H

#include "LuaObject.h"
#include "Context.h"

namespace UI {
namespace Lua {

	void Init();

	// get widget from stack. handles table.widget and autoconstructs Labels
	// from strings too
	UI::Widget *GetWidget(UI::Context *c, lua_State *l, int idx);
	UI::Widget *CheckWidget(UI::Context *c, lua_State *l, int idx);

}
}

#endif
