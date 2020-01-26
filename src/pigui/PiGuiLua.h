// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PIGUI_LUA_H
#define PIGUI_LUA_H

#include "lua/LuaObject.h"

namespace PiGUI {
	void RegisterSandbox();

	namespace Lua {

		void Init();
	}
} // namespace PiGUI

#endif
