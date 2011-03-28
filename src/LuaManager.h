#ifndef _LUAMANAGER_H
#define _LUAMANAGER_H

#include <memory>

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

class LuaManager {
	friend class std::auto_ptr<LuaManager>;

public:
	static LuaManager *Instance()
	{
		if (not s_instance.get()) {
			s_instance = std::auto_ptr<LuaManager>(new LuaManager);
			s_instance.get()->Init();
		}
		return s_instance.get();
	}

	static void Destroy() { return s_instance.reset(); }

	lua_State *GetLuaState() { return m_lua; }

private:
	static std::auto_ptr<LuaManager> s_instance;

	void Init();

	LuaManager();
	~LuaManager();
	LuaManager(const LuaManager &);
	LuaManager &operator=(const LuaManager &);

	lua_State *m_lua;
};

#endif
