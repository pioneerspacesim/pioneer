#ifndef _LUACONSTANTS_H
#define _LUACONSTANTS_H

#include "LuaManager.h"

namespace LuaConstants {
	void Register(lua_State *l);

	inline void Register() {
		Register(LuaManager::Instance()->GetLuaState());
	}
}

#endif
