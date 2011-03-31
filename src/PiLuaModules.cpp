#include "libs.h"
#include "Pi.h"
#include <map>
#include <set>
#include "Object.h"
#include "Body.h"
#include "PiLuaModules.h"
#include "PiLuaAPI.h"
#include "LuaManager.h"
#ifdef _WIN32
#include "win32-dirent.h"
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif


namespace PiLuaModules {

static lua_State *L;
static std::list<std::string> s_modules;

lua_State *GetLuaState() { return L; }

static void GetMission(std::list<Mission> &missions)
{
	LUA_DEBUG_START(L);
	Mission m;
	// mission table at -1
	lua_getfield(L, -1, "description");
	m.m_missionText = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "client");
	m.m_clientName = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "reward");
	m.m_agreedPayoff = lua_tonumber(L, -1)*100.0;
	lua_pop(L, 1);

	lua_getfield(L, -1, "due");
	m.m_dueDate = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "status");
	const char *status = lua_tostring(L, -1);
	if (0 == strcmp(status, "completed")) {
		m.m_status = Mission::COMPLETED;
	} else if (0 == strcmp(status, "failed")) {
		m.m_status = Mission::FAILED;
	} else {
		m.m_status = Mission::ACTIVE;
	}
	lua_pop(L, 1);
	missions.push_back(m);
	LUA_DEBUG_END(L, 0);
}

void GetPlayerMissions(std::list<Mission> &missions)
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		LUA_DEBUG_START(L);
		lua_getglobal(L, (*i).c_str());
		lua_pushcfunction(L, pi_lua_panic);
		lua_getfield(L, -2, "GetPlayerMissions");
		if (!lua_isnil(L, -1)) {
			lua_pushvalue(L, -3); // push self
			lua_pcall(L, 1, 1, -3);
			// -1 is table of missions
			lua_pushnil(L);  /* first key */
			while (lua_next(L, -2) != 0) {
				/* 'key' (at index -2) and 'value' (at index -1) */
				GetMission(missions);
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 3);
		LUA_DEBUG_END(L, 0);
	}
}

}
