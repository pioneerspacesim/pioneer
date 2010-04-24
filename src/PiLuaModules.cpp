#include "libs.h"
#include "Pi.h"
#include "Ship.h"
#include <map>
#include <set>

// only using for including lua headers...
#include "oolua/oolua.h"
#include "oolua/oolua_error.h"

class ObjectWrapper
{
	public:
	ObjectWrapper(): m_obj(0) {}
	ObjectWrapper(Object *o): m_obj(o) {
		m_delCon = o->onDelete.connect(sigc::mem_fun(this, &ObjectWrapper::OnDelete));
	}
	bool IsBody() const {
		return Is(Object::BODY);
	}
	const char *GetLabel() const {
		if (Is(Object::BODY)) {
			return static_cast<Body*>(m_obj)->GetLabel().c_str();
		} else {
			return "";
		}
	}
	int print() { printf("ObjectWrapper = %p;\n", m_obj); return 1; }
	friend bool operator==(const ObjectWrapper &a, const ObjectWrapper &b) {
		return a.m_obj == b.m_obj;
	}
	virtual ~ObjectWrapper() {
	//	printf("ObjWrapper for %s is being deleted\n", GetLabel());
		m_delCon.disconnect();
	}
	protected:
	void OnDelete() {
		// object got deleted out from under us
		m_obj = 0;
		m_delCon.disconnect();
	}
	bool Is(Object::Type t) const {
		return m_obj && m_obj->IsType(t);
	}
	Object *m_obj;
	sigc::connection m_delCon;
};
OOLUA_CLASS_NO_BASES(ObjectWrapper)
	OOLUA_TYPEDEFS
		OOLUA::Equal_op
	OOLUA_END_TYPES
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
//	OOLUA_CONSTRUCTORS_BEGIN
//		OOLUA_CONSTRUCTOR_1(const ObjectWrapper &)
//	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_0(int,print)
	OOLUA_MEM_FUNC_0_CONST(bool, IsBody)
	OOLUA_MEM_FUNC_0_CONST(const char *, GetLabel)
OOLUA_CLASS_END

EXPORT_OOLUA_FUNCTIONS_1_NON_CONST(ObjectWrapper, print)
EXPORT_OOLUA_FUNCTIONS_2_CONST(ObjectWrapper, IsBody, GetLabel)

template <typename T>
static void push2luaWithGc(lua_State *L, T *o)
{
	OOLUA::INTERNAL::Lua_ud* ud = OOLUA::INTERNAL::add_ptr<T>(L,o,false);
	ud->gc = true;
}

namespace PiLuaModules {

static OOLUA::Script *S;
static lua_State *L;
static std::list<std::string> s_modules;
static std::map<std::string, std::set<std::string> > s_eventListeners;
static bool s_isInitted = false;
static bool s_eventsPending = false;

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


namespace LuaFuncs {
	static int PiPlayer(lua_State *const L)
	{
		push2luaWithGc(L, new ObjectWrapper((Object*)Pi::player));
		return 1;
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

void Init()
{
	if (!s_isInitted) {
		s_isInitted = true;

		OOLUA::Script *S = new OOLUA::Script;
		S->register_class<ObjectWrapper>();
		L = S->get_ptr();
	//	L = lua_open();
		luaL_openlibs(L);
		lua_register(L, "PiPlayer", LuaFuncs::PiPlayer);
		lua_register(L, "PiModule", register_module);

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
