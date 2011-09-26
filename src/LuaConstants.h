#ifndef _LUACONSTANTS_H
#define _LUACONSTANTS_H

#include "Pi.h"

namespace LuaConstants {
	void Register(lua_State *l);
	inline void Register() {
		Register(Pi::luaManager->GetLuaState());
	}

    int GetConstant(lua_State *l, const char *ns, const char *name);
    inline int GetConstant(const char *ns, const char *name) {
		return GetConstant(Pi::luaManager->GetLuaState(), ns, name);
	}

    const char *GetConstantString(lua_State *l, const char *ns, int value);
    inline const char *GetConstantString(const char *ns, int value) {
		return GetConstantString(Pi::luaManager->GetLuaState(), ns, value);
	}
}

#endif
