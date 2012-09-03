#ifndef _LUA_H
#define _LUA_H

#include "LuaManager.h"

// home for the global Lua context. here so its shareable between pioneer and
// modelviewer. probably sucks in the long term
namespace Lua {

extern LuaManager *manager;

void Init();
void Uninit();

}

#endif
