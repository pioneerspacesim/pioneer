#include "libs.h"
// only using for including lua headers...
#include "MyLuaMathTypes.h"
#include "Pi.h"
#include <map>
#include <set>

namespace PiLuaModules {

static lua_State *L;
static std::list<std::string> s_modules;
static std::map<std::string, std::set<std::string> > s_eventListeners;
static bool s_isInitted = false;

static void CallModFunction(const char *modname, const char *funcname)
{
	lua_getglobal(L, modname);
	lua_getfield(L, -1, funcname);
	lua_pushvalue(L, -2); // push self
	lua_call(L, 1, 0);
	lua_pop(L, 2);
}

static void ModsInitAll()
{
	for(std::list<std::string>::const_iterator i = s_modules.begin(); i!=s_modules.end(); ++i) {
		printf("Calling %s:Init()\n", (*i).c_str());
		CallModFunction((*i).c_str(), "Init");
	}
}


namespace LuaFuncs {
	static int EventListen(lua_State * const L)
	{
		int argmax = lua_gettop(L);
		lua_getfield(L, 1, "__name");
		const char *modname = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		printf("::EventListen() for %s\n", modname);
		for (int i=2; i<=argmax; i++) {
			const char *event_name = luaL_checkstring(L, i);
			printf("Listen to %s\n", event_name);
			//std::map<std::string, std::list<std::string> >::iterator eventHearList;
			//eventHearList = s_eventListeners.find(event_name);
			s_eventListeners[event_name].insert(modname);
		}
	}
	
	static int EventIgnore(lua_State * const L)
	{
		int argmax = lua_gettop(L);
		lua_getfield(L, 1, "__name");
		const char *modname = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		printf("::EventIgnore() for %s\n", modname);
		for (int i=2; i<=argmax; i++) {
			const char *event_name = luaL_checkstring(L, i);
			//std::map<std::string, std::list<std::string> >::iterator eventHearList;
			//eventHearList = s_eventListeners.find(event_name);
			s_eventListeners[event_name].erase(modname);
		}
	}
}

static void mods_event_dispatcher(const char *event)
{
	std::map<std::string, std::set<std::string> >::iterator eventListeners;
	eventListeners = s_eventListeners.find(event);

	if (eventListeners != s_eventListeners.end()) {
		std::set<std::string> &turd = (*eventListeners).second;
		for (std::set<std::string>::iterator i = turd.begin(); i!=turd.end(); ++i) {
			CallModFunction((*i).c_str(), event);
		}
	}
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

void Init()
{
	if (!s_isInitted) {
		s_isInitted = true;
		L = lua_open();
		luaL_openlibs(L);
		lua_register(L, "PiModule", register_module);
		lua_register(L, "EventListen", LuaFuncs::EventListen);
		lua_register(L, "EventIgnore", LuaFuncs::EventIgnore);

		if (luaL_dofile(L, "test_module.lua")) {
			Error("%s", lua_tostring(L, -1));
		}
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
