#ifndef _PILUAMODULES_H
#define _PILUAMODULES_H

#include "Mission.h"
#include "Serializer.h"
#include "LuaManager.h"
#include "LuaUtils.h"

#include "oolua/oolua.h"

namespace PiLuaModules {
	void GetPlayerMissions(std::list<Mission> &missions);
	void UpdateOncePerRealtimeSecond();
	lua_State *GetLuaState();

	void Init();
	void Uninit();

	void Serialize(Serializer::Writer &wr);
	void Unserialize(Serializer::Reader &rd);

	/* ----------- The ugly shit -------------- */
	static inline void ModCall(const char *modname, const char *fnname, int retvals)
	{
		lua_State *l = GetLuaState();
		LUA_DEBUG_START(l)
		lua_pushcfunction(l, pi_lua_panic);
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		lua_pcall(l, 1, retvals, -3);
		lua_remove(l, -1-retvals); // get rid of pi_lua_panic
		LUA_DEBUG_END(l, retvals);
	}
	template <typename T>
	static inline void ModCall(const char *modname, const char *fnname, int retvals, T arg1)
	{
		lua_State *l = PiLuaModules::GetLuaState();
		LUA_DEBUG_START(l)
		lua_pushcfunction(l, pi_lua_panic);
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		OOLUA::push2lua(l, arg1);
		lua_pcall(l, 2, retvals, -4);
		lua_remove(l, -1-retvals); // get rid of pi_lua_panic
		LUA_DEBUG_END(l, retvals);
	}
	template <typename T, typename U>
	static inline void ModCall(const char *modname, const char *fnname, int retvals, T arg1, U arg2)
	{
		lua_State *l = PiLuaModules::GetLuaState();
		LUA_DEBUG_START(l)
		lua_pushcfunction(l, pi_lua_panic);
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		OOLUA::push2lua(l, arg1);
		OOLUA::push2lua(l, arg2);
		lua_pcall(l, 3, retvals, -5);
		lua_remove(l, -1-retvals); // get rid of pi_lua_panic
		LUA_DEBUG_END(l, retvals);
	}
	template <typename T, typename U, typename V>
	static inline void ModCall(const char *modname, const char *fnname, int retvals, T arg1, U arg2, V arg3)
	{
		lua_State *l = PiLuaModules::GetLuaState();
		LUA_DEBUG_START(l)
		lua_pushcfunction(l, pi_lua_panic);
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		OOLUA::push2lua(l, arg1);
		OOLUA::push2lua(l, arg2);
		OOLUA::push2lua(l, arg3);
		lua_pcall(l, 4, retvals, -6);
		lua_remove(l, -1-retvals); // get rid of pi_lua_panic
		LUA_DEBUG_END(l, retvals);
	}

}

#endif /* _PILUAMODULES_H */
