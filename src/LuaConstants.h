#ifndef _LUACONSTANTS_H
#define _LUACONSTANTS_H

#include "LuaManager.h"

namespace LuaConstants {
	void RegisterConstants(lua_State *l);

	inline void RegisterConstants() {
		RegisterConstants(LuaManager::Instance()->GetLuaState());
	}
}

#endif
