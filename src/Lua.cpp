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
