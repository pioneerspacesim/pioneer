// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"

namespace Lua {

LuaManager *manager = 0;

void Init()
{
	manager = new LuaManager();
}

void Uninit()
{
	delete manager;
	manager = 0;
}

}
