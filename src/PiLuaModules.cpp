#include "libs.h"
#include "Pi.h"
#include <map>
#include <set>
#include "Object.h"
#include "Body.h"
#include "PiLuaModules.h"
#include "mylua.h"
#include "PiLuaAPI.h"
#if 0
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
static std::map<std::string, std::set<std::string> > s_eventListeners;
static bool s_isInitted = false;
static bool s_eventsPending = false;

lua_State *GetLuaState() { return L; }

void UpdateOncePerRealtimeSecond()
{
	LUA_DEBUG_START(L);
	lua_pushcfunction(L, mylua_panic);
	lua_getglobal(L, "UpdateOncePerRealtimeSecond");
	lua_pcall(L, 0, 0, -2);
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0)
}

void EmitEvents()
{
	if (s_eventsPending) {
		LUA_DEBUG_START(L)
		lua_pushcfunction(L, mylua_panic);
		lua_getglobal(L, "EmitEvents");
		lua_pcall(L, 0, 0, -2);
		lua_pop(L, 1);
		LUA_DEBUG_END(L, 0)
		s_eventsPending = false;
	}
}

void QueueEvent(const char *eventName)
{
	LUA_DEBUG_START(L)
	s_eventsPending = true;
	lua_getglobal(L, "__pendingEvents");
	size_t len = lua_objlen (L, -1);
	lua_pushinteger(L, len+1);
	
	// create event: { type=>eventName, 1=>o1,2=>o2 }
	lua_createtable (L, 2, 1);
	lua_pushstring(L, eventName);
	lua_setfield(L, -2, "type");

	// insert event into __pendingEvents
	lua_settable(L, -3);
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0)
}

void QueueEvent(const char *eventName, Object *o1)
{
	LUA_DEBUG_START(L)
	s_eventsPending = true;
	lua_getglobal(L, "__pendingEvents");
	size_t len = lua_objlen (L, -1);
	lua_pushinteger(L, len+1);
	
	// create event: { type=>eventName, 1=>o1,2=>o2 }
	lua_createtable (L, 2, 1);
	lua_pushstring(L, eventName);
	lua_setfield(L, -2, "type");

	lua_pushinteger(L, 1);
	OOLUA::push2lua(L, o1);
	lua_settable(L, -3);
	
	// insert event into __pendingEvents
	lua_settable(L, -3);
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0)
}

void QueueEvent(const char *eventName, Object *o1, Object *o2)
{
	LUA_DEBUG_START(L)
	s_eventsPending = true;
	lua_getglobal(L, "__pendingEvents");
	size_t len = lua_objlen (L, -1);
	lua_pushinteger(L, len+1);
	
	// create event: { type=>eventName, 1=>o1,2=>o2 }
	lua_createtable (L, 2, 1);
	lua_pushstring(L, eventName);
	lua_setfield(L, -2, "type");

	lua_pushinteger(L, 1);
	OOLUA::push2lua(L, o1);
	lua_settable(L, -3);
	
	lua_pushinteger(L, 2);
	OOLUA::push2lua(L, o2);
	lua_settable(L, -3);
	
	// insert event into __pendingEvents
	lua_settable(L, -3);
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0)
}

static void CallModFunction(const char *modname, const char *funcname)
{
	LUA_DEBUG_START(L)
	//printf("Calling %s:%s()\n", modname, funcname);
	lua_pushcfunction(L, mylua_panic);
	lua_getglobal(L, modname);
	lua_getfield(L, -1, funcname);
	lua_pushvalue(L, -2); // push self
	lua_pcall(L, 1, 0, -4);
	lua_pop(L, 2);
	LUA_DEBUG_END(L, 0)
}

static void ModsInitAll()
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		CallModFunction((*i).c_str(), "Init");
	}
}

static void GetMission(std::list<Mission> &missions)
{
	LUA_DEBUG_START(L)
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
	LUA_DEBUG_END(L, 0)
}

void GetPlayerMissions(std::list<Mission> &missions)
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		LUA_DEBUG_START(L)
		lua_getglobal(L, (*i).c_str());
		lua_pushcfunction(L, mylua_panic);
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
		LUA_DEBUG_END(L, 0)
	}
}

void Serialize(Serializer::Writer &wr)
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		ModCall((*i).c_str(), "Serialize", 1);
		// string can have nulls in it so must get length
		std::string str;
		OOLUA::pull2cpp(L, str);
		wr.String(*i);
		wr.String(str);
	}
	wr.String("");
}

void Unserialize(Serializer::Reader &rd)
{
	// XXX TODO XXX keep saved data for modules not enabled,
	// so we can re-save it an not lose it
	std::string modname;
	std::string moddata;

	for (;;) {
		modname = rd.String();
		if (modname == "") break;
		moddata = rd.String();
		bool found = false;
		for (std::list<std::string>::const_iterator i = s_modules.begin(); i != s_modules.end(); ++i) {
			if ((*i) == modname) {
				ModCall(modname.c_str(), "Unserialize", 0, moddata);
				found = true;
				break;
			}
		}
		if (!found) {
			// XXX this isn't good. we should keep the data for re-saving in case the module is enabled again
			Warning("Could not find the module '%s'. Continuing, but module data will be lost.", modname.c_str());
		}
	}
}

static void mods_event_dispatcher(const char *event)
{
	QueueEvent(event);
}

static int register_module(lua_State * const L)
{
	if (!lua_istable(L, 1)) {
		luaL_error(L, "register_module passed incorrect arguments");
	} else {
		lua_getfield(L, 1, "__name");
		const char *module_name = luaL_checkstring(L, -1);
		printf("mod: %s\n", module_name);
		lua_pop(L, 1);
		
		lua_pushnil(L);  /* first key */
		while (lua_next(L, 1) != 0) {
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			if (lua_isstring(L, -2)) {
				std::string key = lua_tostring(L, -2);
			//	printf("(%s): %s - %s\n", key.c_str(),
			//		lua_typename(L, lua_type(L, -2)),
			//		lua_typename(L, lua_type(L, -1)));
			}
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);
		}
		
		lua_pushvalue(L, 1);
	//	char buf[256];
	//	snprintf(buf, sizeof(buf), "module_%s", module_name);
		lua_setglobal(L, module_name);
		s_modules.push_back(module_name);
	}
	return 0;
}

static void DoAllLuaModuleFiles(lua_State *L)
{
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir("data/modules"))) {
		Error("Could not open data/modules");
	} else {
		while ((entry = readdir(dir)) != 0) {
			if (entry->d_name[0] == '.') continue;
			// only want to execute .lua files
			if (strcmp(".lua", &entry->d_name[ strlen(entry->d_name)-4 ])) {
				continue;
			}
			std::string path = "data/modules/" + std::string(entry->d_name);
			//printf("Running %s\n", path.c_str());
			if (luaL_dofile(L, path.c_str())) {
				Error("%s", lua_tostring(L, -1));
			}
		}
		closedir(dir);
	}
}

void Init()
{
	if (!s_isInitted) {
		s_isInitted = true;

		OOLUA::Script *S = new OOLUA::Script;
		L = S->get_ptr();
		luaL_openlibs(L);
		lua_register(L, "PiModule", register_module);

		RegisterPiLuaAPI(L);

		if (luaL_dofile(L, "data/pimodule.lua")) {
			Error("%s", lua_tostring(L, -1));
		}
		DoAllLuaModuleFiles(L);

		ModsInitAll();
		Pi::onPlayerChangeTarget.connect(sigc::bind(sigc::ptr_fun(&mods_event_dispatcher), "onPlayerChangeTarget"));
	}
}

void Uninit()
{
	if (s_isInitted) {
		s_isInitted = false;
		lua_close(L);
		s_modules.clear();
		s_eventListeners.clear();
	}
}

}
