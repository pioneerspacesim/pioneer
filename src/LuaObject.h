// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include "Lua.h"
#include "LuaRef.h"
#include "LuaPushPull.h"
#include "LuaUtils.h"
#include "LuaWrappable.h"
#include "RefCounted.h"
#include "DeleteEmitter.h"
#include <typeinfo>
#include <tuple>

//
// LuaObject provides proxy objects and tracking facilities to safely get
// objects in and out of lua. the basic idea is that for every class you want
// to expose to lua, you define LuaObject wrapper class that defines the
// lua name lua name, methods and metamethods for that class. you then call
// methods on this class to push and pull objects to and from the lua stack
//
// Push an object to the Lua stack:
//
//   // C++-owned, still responsible for deletion
//   Ship *s = new Ship("wave");
//   LuaObject<Ship>::PushToLua(s);
//
//   // RefCounted, Lua will take a reference
//   StarSystem *s = Pi::GetGalaxy()->GetStarSystem(SystemPath(0,0,0,0));
//   LuaObject<StarSystem>::PushToLua(s);
//
//   // Heap-allocated, Lua will get a copy
//   SystemPath path(0,0,0,0,1);
//   LuaObject<SystamPath>::PushToLua(path);
//
// Get an object from the Lua stack at index n. Causes a Lua exception if the
// object doesn't exist or the types don't match.
//
//   Ship *s = LuaObject<Ship>::CheckFromLua(1);
//
// Or alternatively, get it and return 0 on failure.
//
//   Ship *s = LuaObject<Ship>::GetFromLua(1);
//
//
// If you need to expose a new class to Lua:
//
// - Have it inherit from LuaWrappable
//
// - Have it either:
//   - inherit from DeleteEmitter
//   - inherit from RefCounted
//   - implement a copy constructor
//
// - Arrange for the wrapper class RegisterClass() method to be called in
//   LuaInit in Pi.cpp
//
// - Make a new file LuaWhatever.cpp implement RegisterClass() and any
//   methods, metamethods and attribute methods you want. Copy from one of the
//   other files to get the idea
//
// - Add the new file to the build system
//


// type for promotion test callbacks
typedef bool (*PromotionTest)(LuaWrappable *o);


// wrapper baseclass, and extra bits for getting at certain parts of the
// LuaObject layer 
class LuaObjectBase {
	friend class LuaSerializer;

public:
	// creates a single "typeless" object and attaches the listed methods,
	// attributes and metamethods to it. leaves the created object on the
	// stack
	static void CreateObject(const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta, bool protect = false);

	// get all valid method/attribute names for the object on the top of the
	// stack. mainly intended for the console. uses the same logic as the
	// method dispatcher
	static void GetNames(std::vector<std::string> &names, const std::string &prefix = "", bool methodsOnly = false);

protected:
	// base class constructor, called by the wrapper Push* methods
	LuaObjectBase(const char *type) : m_type(type) {};
	virtual ~LuaObjectBase() {}

	// creates a class in the lua vm with the given name and attaches the
	// listed methods to it and the listed metamethods to its metaclass. if
	// attributes extra magic is added to the metaclass to make them work as
	// expected
	static void CreateClass(const char *type, const char *parent, const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta);

	// push an already-registered object onto the lua stack. the object is
	// looked up in the lua registry, if it exists its userdata its userdata
	// is pushed onto the lua stack. returns true if the object exists and was
	// pushed, false otherwise
	static bool PushRegistered(LuaWrappable *o);

	// adds an object->wrapper mapping to the registry for the given wrapper
	// object. the wrapper's corresponding userdata should be on the top of
	// the stack
	static void Register(LuaObjectBase *lo);

	// remove the object->wrapper from the registry. checks to make sure the
	// the mapping matches first, to protect against memory being reused
	static void Deregister(LuaObjectBase *lo);

	// pulls an object off the lua stack and returns its associated c++
	// object. type is the lua type string of the object. a lua exception is
	// triggered if the object on the stack is not of this type
	static LuaWrappable *CheckFromLua(int index, const char *type);

	// does exactly the same as Check without triggering exceptions
	static LuaWrappable *GetFromLua(int index, const char *type);

	// register a promotion test. when an object with lua type base_type is
	// pushed, test_fn will be called. if it returns true then the created lua
	// object will be of target_type
	static void RegisterPromotion(const char *base_type, const char *target_type, PromotionTest test_fn);

    // allocate n bytes from Lua memory and leave it an associated userdata on
    // the stack. this is a wrapper around lua_newuserdata
	static void *Allocate(size_t n);

    // get a pointer to the underlying object
	virtual LuaWrappable *GetObject() const = 0;

	const char *GetType() const { return m_type; }

private:
	LuaObjectBase() {}
	LuaObjectBase(const LuaObjectBase &) {}

	// lua method to determine if the underlying object is still present in
	// the registry (ie still exists)
	static int l_exists(lua_State *l);

	// lua method to determine if the object inherits from a type. wrapper
	// around ::Isa()
	static int l_isa(lua_State *l);

	// lua method to set a property on a propertied object
	static int l_setprop(lua_State *l);

	// lua method to unset a property on a propertied object
	static int l_unsetprop(lua_State *l);

	// lua method to check the existence of a specific property on an object
	static int l_hasprop(lua_State *l);

	// the lua object "destructor" that gets called by the garbage collector.
	static int l_gc(lua_State *l);

	// default tostring. shows a little more info about the object, like its
	// type
	static int l_tostring(lua_State *l);

	// __index metamethod
	static int l_dispatch_index(lua_State *l);

    // determine if the object has a class in its ancestry
    bool Isa(const char *base) const;

	// lua type (ie method/metatable name)
	const char *m_type;
};


// templated portion of the wrapper baseclass
template <typename T>
class LuaObject : public LuaObjectBase {
public:

	// registers the class with the lua vm
	static void RegisterClass();

	// wrap an object and push it onto the stack. these create a wrapper
	// object that knows how to deal with the type of object
	static inline void PushToLua(DeleteEmitter *o); // LuaCoreObject
	static inline void PushToLua(RefCounted *o);    // LuaSharedObject
	static inline void PushToLua(const T &o);       // LuaCopyObject

	template <typename Ret, typename Key, typename ...Args>
		static inline Ret CallMethod(T* o, const Key &key, const Args &...args);
	template <typename Key, typename ...Args>
		static inline void CallMethod(T* o, const Key &key, const Args &...args) {
			CallMethod<bool>(o, key, args...);
		}

	template <typename Ret1, typename Ret2, typename ...Ret, typename Key, typename ...Args>
		static inline std::tuple<Ret1, Ret2, Ret...> CallMethod(T* o, const Key &key, const Args &...args);

	// pull an object off the stack, unwrap and return it
	// if not found or doesn't match the type, throws a lua exception
	static inline T *CheckFromLua(int idx) {
		return dynamic_cast<T*>(LuaObjectBase::CheckFromLua(idx, s_type));
	}

	// same but without error checks. returns 0 on failure
	static inline T *GetFromLua(int idx) {
		return dynamic_cast<T*>(LuaObjectBase::GetFromLua(idx, s_type));
	}

	// standard cast promotion test for convenience
	static inline bool DynamicCastPromotionTest(LuaWrappable *o) {
		return dynamic_cast<T*>(o);
	}

protected:
	LuaObject() : LuaObjectBase(s_type) {}

private:

	// initial lua type string. defined in a specialisation in the appropriate
	// .cpp file
	static const char *s_type;
};


// wrapper for a "core" object - one owned by c++ (eg Body).
// Lua needs to know when the object is deleted so that it can handle
// requests for it appropriately (ie with an exception, or exists())
template <typename T>
class LuaCoreObject : public LuaObject<T> {
public:
	LuaCoreObject(T *o) : m_object(o) {
		m_deleteConnection = m_object->DeleteEmitter::onDelete.connect(sigc::mem_fun(this, &LuaCoreObject::OnDelete));
	}

	~LuaCoreObject() {
		if (m_deleteConnection.connected())
			m_deleteConnection.disconnect();
	}

	LuaWrappable *GetObject() const {
		return m_object;
	}

private:
	void OnDelete() {
		LuaObjectBase::Deregister(this);
		m_object = 0;
	}

	T *m_object;
	sigc::connection m_deleteConnection;
};


// wrapper for a "shared" object - one that can comfortably exist in both
// environments. usually for long-lived (StarSystem) or standalone (UI
// widget) objects
// Lua simply needs to keep a reference to these
template <typename T>
class LuaSharedObject : public LuaObject<T> {
public:
	LuaSharedObject(T *o) : m_object(o) {}

	LuaWrappable *GetObject() const {
		return m_object.Get();
	}

private:
	RefCountedPtr<T> m_object;
};


// wrapper for a "copied" object. a new one is created via the copy
// constructor and fully owned by Lua. good for lightweight POD-style objects
// (eg SystemPath)
template <typename T>
class LuaCopyObject : public LuaObject<T> {
public:
	LuaCopyObject(const T &o) {
		lua_State *l = Lua::manager->GetLuaState();
		m_object = new (LuaObjectBase::Allocate(sizeof(T))) T(o);
		m_ref = LuaRef(l, -1);
		lua_pop(l, 1);
	}

	~LuaCopyObject() {
		m_object->~T();
		m_object = 0;
	}

	LuaWrappable *GetObject() const {
		return m_object;
	}

private:
	T *m_object;
	LuaRef m_ref;
};


// push methods, create wrappers if necessary
// wrappers are allocated from Lua memory
template <typename T> inline void LuaObject<T>::PushToLua(DeleteEmitter *o) {
	if (!PushRegistered(o))
		Register(new (LuaObjectBase::Allocate(sizeof(LuaCoreObject<T>))) LuaCoreObject<T>(static_cast<T*>(o)));
}

template <typename T> inline void LuaObject<T>::PushToLua(RefCounted *o) {
	if (!PushRegistered(o))
		Register(new (LuaObjectBase::Allocate(sizeof(LuaSharedObject<T>))) LuaSharedObject<T>(static_cast<T*>(o)));
}

template <typename T> inline void LuaObject<T>::PushToLua(const T &o) {
	Register(new (LuaObjectBase::Allocate(sizeof(LuaCopyObject<T>))) LuaCopyObject<T>(o));
}

template <typename T>
template <typename Ret, typename Key, typename ...Args>
inline Ret LuaObject<T>::CallMethod(T* o, const Key &key, const Args &...args) {
	lua_State *l = Lua::manager->GetLuaState();
	LUA_DEBUG_START(l);
	Ret return_value;

	lua_checkstack(l, sizeof...(args)+5);
	PushToLua(o);
	pi_lua_generic_push(l, key);
	lua_gettable(l, -2);
	lua_pushvalue(l, -2);
	lua_remove(l, -3);
	pi_lua_multiple_push(l, args...);
	pi_lua_protected_call(l, sizeof...(args)+1, 1);
	pi_lua_generic_pull(l, -1, return_value);
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
	return return_value;
}

template <typename T>
template <typename Ret1, typename Ret2, typename ...Ret, typename Key, typename ...Args>
inline std::tuple<Ret1, Ret2, Ret...> LuaObject<T>::CallMethod(T* o, const Key &key, const Args &...args) {
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);
	lua_checkstack(l, sizeof...(args)+5);
	PushToLua(o);
	pi_lua_generic_push(l, key);
	lua_gettable(l, -2);
	lua_pushvalue(l, -2);
	lua_remove(l, -3);
	pi_lua_multiple_push(l, args...);
	pi_lua_protected_call(l, sizeof...(args)+1, 2+sizeof...(Ret));
	auto ret_values = pi_lua_multiple_pull<Ret1, Ret2, Ret...>(l, -2-static_cast<int>(sizeof...(Ret)));
	lua_pop(l, 2+static_cast<int>(sizeof...(Ret)));
	LUA_DEBUG_END(l, 0);
	return ret_values;
}

// specialise for SystemPath, which needs custom machinery to deduplicate system paths
class SystemPath;
template <> void LuaObject<SystemPath>::PushToLua(const SystemPath &o);

// LuaPushPull stuff.
template <class T> void pi_lua_generic_pull(lua_State * l, int index, T* & out) {
	assert(l == Lua::manager->GetLuaState());
	out = LuaObject<T>::CheckFromLua(index);
}

template <class T> bool pi_lua_strict_pull(lua_State * l, int index, T* & out) {
	assert(l == Lua::manager->GetLuaState());
	out = LuaObject<T>::GetFromLua(index);
	return out != 0;
}

template <class T> void pi_lua_generic_push(lua_State * l, T* value) {
	assert(l == Lua::manager->GetLuaState());
	if (value)
		LuaObject<T>::PushToLua(value);
	else
		lua_pushnil(l);
}

#endif
