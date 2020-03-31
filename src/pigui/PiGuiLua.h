// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PIGUI_LUA_H
#define PIGUI_LUA_H

#include "lua/LuaObject.h"

namespace PiGUI {

	// Get registered PiGui handlers.
	LuaRef GetHandlers();
	// Get a table of key name to SDL-keycode mappings
	LuaRef GetKeys();

	namespace Lua {
		void RegisterSandbox();

		void Init();
		void Uninit();
	} // namespace Lua
} // namespace PiGUI

#endif
