// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUA_H
#define _LUA_H

#include "LuaManager.h"

// home for the global Lua context. here so its shareable between pioneer and
// modelviewer. probably sucks in the long term
namespace Lua {

	extern LuaManager *manager;

	// Initialize the lua instance
	void Init();
	// Uninitialize the lua instance
	void Uninit();

	// Initialize all lua bindings and modules
	void InitModules();
	// Uninitialize all lua bindings and modules
	void UninitModules();

} // namespace Lua

#endif
