// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

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
