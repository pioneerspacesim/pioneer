#ifndef _LUACONSTANTS_H
#define _LUACONSTANTS_H

#include "Pi.h"

namespace LuaConstants {
	void Register(lua_State *l);

	inline void Register() {
		Register(Pi::luaManager.GetLuaState());
	}
}

#endif
