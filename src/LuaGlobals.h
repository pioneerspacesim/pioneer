#ifndef _LUAGLOBALS_H
#define _LUAGLOBALS_H

#include "LuaManager.h"

namespace LuaGlobals {
	void RegisterConstants(lua_State *l);

	inline void RegisterConstants() {
		RegisterConstants(LuaManager::Instance()->GetLuaState());
	}
}

#endif
