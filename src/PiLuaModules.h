#ifndef _PILUAMODULES_H
#define _PILUAMODULES_H

#include "Mission.h"
#include "oolua/oolua.h"

namespace PiLuaModules {
	void GetPlayerMissions(std::list<Mission> &missions);
	void QueueEvent(const char *eventName);
	void QueueEvent(const char *eventName, Object *o1);
	void QueueEvent(const char *eventName, Object *o1, Object *o2);
	void EmitEvents();
	lua_State *GetLuaState();

	void Init();
	void Uninit();

	void Serialize();
	void Unserialize();

	/* ----------- The ugly shit -------------- */
	static inline void ModCall(const char *modname, const char *fnname, int retvals)
	{
		lua_State *l = GetLuaState();
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		lua_call(l, 1, retvals);
	}
	template <typename T>
	static inline void ModCall(const char *modname, const char *fnname, int retvals, T arg1)
	{
		lua_State *l = PiLuaModules::GetLuaState();
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		OOLUA::push2lua(l, arg1);
		lua_call(l, 2, retvals);
	}
	template <typename T, typename U>
	static inline void ModCall(const char *modname, const char *fnname, int retvals, T arg1, U arg2)
	{
		lua_State *l = PiLuaModules::GetLuaState();
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		OOLUA::push2lua(l, arg1);
		OOLUA::push2lua(l, arg2);
		lua_call(l, 3, retvals);
	}
	template <typename T, typename U, typename V>
	static inline void ModCall(const char *modname, const char *fnname, int retvals, T arg1, U arg2, V arg3)
	{
		lua_State *l = PiLuaModules::GetLuaState();
		lua_getglobal(l, modname);
		lua_getfield(l, -1, fnname);
		lua_pushvalue(l, -2); // push self
		lua_remove(l, -3); // get rid of module object
		OOLUA::push2lua(l, arg1);
		OOLUA::push2lua(l, arg2);
		OOLUA::push2lua(l, arg3);
		lua_call(l, 4, retvals);
	}

}

#endif /* _PILUAMODULES_H */
