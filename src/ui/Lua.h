// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_LUA_H
#define UI_LUA_H

#include "LuaObject.h"
#include "Context.h"

namespace UI {
namespace Lua {

	void Init();

	// get widget from stack. handles table.widget format as well
	UI::Widget *GetWidget(lua_State *l, int idx);
	UI::Widget *CheckWidget(lua_State *l, int idx);

}
}

#endif
