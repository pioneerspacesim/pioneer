#include "libs.h"
#include "Pi.h"
#include <map>
#include <set>
#include "Object.h"
#include "Body.h"
#include "Serializer.h"
#include "PiLuaModules.h"
#include "mylua.h"
#include "PiLuaAPI.h"
#ifdef _WIN32
#include <../msvc/win32-dirent.h>
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

void EmitEvents()
{
	if (s_eventsPending) {
		lua_getglobal(L, "EmitEvents");
		lua_call(L, 0, 0);
		s_eventsPending = false;
	}
}

void QueueEvent(const char *eventName)
{
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
}

void QueueEvent(const char *eventName, Object *o1)
{
	s_eventsPending = true;
	lua_getglobal(L, "__pendingEvents");
	size_t len = lua_objlen (L, -1);
	lua_pushinteger(L, len+1);
	
	// create event: { type=>eventName, 1=>o1,2=>o2 }
	lua_createtable (L, 2, 1);
	lua_pushstring(L, eventName);
	lua_setfield(L, -2, "type");

	lua_pushinteger(L, 1);
	push2luaWithGc(L, new ObjectWrapper(o1));
	lua_settable(L, -3);
	
	// insert event into __pendingEvents
	lua_settable(L, -3);
	lua_pop(L, 1);
}

void QueueEvent(const char *eventName, Object *o1, Object *o2)
{
	s_eventsPending = true;
	lua_getglobal(L, "__pendingEvents");
	size_t len = lua_objlen (L, -1);
	lua_pushinteger(L, len+1);
	
	// create event: { type=>eventName, 1=>o1,2=>o2 }
	lua_createtable (L, 2, 1);
	lua_pushstring(L, eventName);
	lua_setfield(L, -2, "type");

	lua_pushinteger(L, 1);
	push2luaWithGc(L, new ObjectWrapper(o1));
	lua_settable(L, -3);
	
	lua_pushinteger(L, 2);
	push2luaWithGc(L, new ObjectWrapper(o2));
	lua_settable(L, -3);
	
	// insert event into __pendingEvents
	lua_settable(L, -3);
	lua_pop(L, 1);
}

static void CallModFunction(const char *modname, const char *funcname)
{
	printf("call %s:%s()\n", modname, funcname);
	lua_getglobal(L, modname);
	lua_getfield(L, -1, funcname);
	lua_pushvalue(L, -2); // push self
	lua_call(L, 1, 0);
	lua_pop(L, 1);
}

static void ModsInitAll()
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		printf("Calling %s:Init()\n", (*i).c_str());
		CallModFunction((*i).c_str(), "Init");
	}
}

static void GetMission(std::list<Mission> &missions)
{
	Mission m;
	// mission table at -1
	lua_getfield(L, -1, "description");
	m.m_missionText = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "client");
	m.m_clientName = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "reward");
	m.m_agreedPayoff = luaL_checknumber(L, -1)*100.0;
	lua_pop(L, 1);

	lua_getfield(L, -1, "status");
	const char *status = luaL_checkstring(L, -1);
	if (0 == strcmp(status, "completed")) {
		m.m_status = Mission::COMPLETED;
	} else if (0 == strcmp(status, "failed")) {
		m.m_status = Mission::FAILED;
	} else {
		m.m_status = Mission::ACTIVE;
	}
	lua_pop(L, 1);
	missions.push_back(m);
}

void GetPlayerMissions(std::list<Mission> &missions)
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		lua_getglobal(L, (*i).c_str());
		lua_getfield(L, -1, "GetPlayerMissions");
		if (!lua_isnil(L, -1)) {
			lua_pushvalue(L, -2); // push self
			lua_call(L, 1, 1);
			// -1 is table of missions
			lua_pushnil(L);  /* first key */
			while (lua_next(L, -2) != 0) {
				/* 'key' (at index -2) and 'value' (at index -1) */
				GetMission(missions);
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		} else {
			lua_pop(L, 2);
		}
	}
}

void Serialize()
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		ModCall((*i).c_str(), "Serialize", 1);
		const char *str = luaL_checkstring(L, -1);
		Serializer::Write::wr_string((*i).c_str());
		Serializer::Write::wr_string(str);
		lua_pop(L, 1);
	}
	Serializer::Write::wr_string("");
}

void Unserialize()
{
	// XXX TODO XXX keep saved data for modules not enabled,
	// so we can re-save it an not lose it
	std::string modname;
	std::string moddata;

	for (;;) {
		modname = Serializer::Read::rd_string();
		if (modname == "") break;
		moddata = Serializer::Read::rd_string();
		
		ModCall(modname.c_str(), "Unserialize", 0, moddata.c_str());
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
				printf("(%s): %s - %s\n", key.c_str(),
					lua_typename(L, lua_type(L, -2)),
					lua_typename(L, lua_type(L, -1)));
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

static int UserDataSerialize(lua_State *L)
{
	ObjectWrapper *o;
	luaL_checktype(L, 1, LUA_TUSERDATA);
	if (mylua_checkudata(L, 1, "ObjectWrapper")) {
		OOLUA::pull2cpp(L, o);
		char buf[128];
		// XXX this is a rather hairy cast but should always be true
		assert(static_cast<ObjectWrapper*>(o)->IsBody());
		snprintf(buf, sizeof(buf), "o%d\n", Serializer::LookupBody((Body*)static_cast<ObjectWrapper*>(o)->m_obj));
		lua_pushstring(L, buf);
		return 1;
	} else {
		Error("Tried to serialize unknown userdata type.");
		return 0;
	}
}

static int UserDataUnserialize(lua_State *L)
{
	size_t idx = atoi(luaL_checkstring(L, 1));
	Body *b = Serializer::LookupBody(idx);
	push2luaWithGc(L, new ObjectWrapper(b));
	return 1;
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
			printf("Running %s\n", path.c_str());
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
		lua_register(L, "UserDataSerialize", UserDataSerialize);
		lua_register(L, "UserDataUnserialize", UserDataUnserialize);

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
