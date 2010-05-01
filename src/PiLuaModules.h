#ifndef _PILUAMODULES_H
#define _PILUAMODULES_H

#include "Mission.h"

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
}

#endif /* _PILUAMODULES_H */
