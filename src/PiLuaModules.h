#ifndef _PILUAMODULES_H
#define _PILUAMODULES_H

#include "Mission.h"
#include "Serializer.h"
#include "LuaManager.h"
#include "LuaUtils.h"

#include "oolua/oolua.h"

namespace PiLuaModules {
	void GetPlayerMissions(std::list<Mission> &missions);
	lua_State *GetLuaState();

	void Serialize(Serializer::Writer &wr);
	void Unserialize(Serializer::Reader &rd);
}

#endif /* _PILUAMODULES_H */
