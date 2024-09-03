// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include "DeleteEmitter.h"
#include "Lua.h"
#include "LuaPushPull.h"
#include "LuaRef.h"
#include "LuaUtils.h"
#include "LuaWrappable.h"
#include "RefCounted.h"
#include "galaxy/SystemPath.h"
#include <tuple>
#include <typeinfo>

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
//   // Stack-allocated, Lua will get a copy
//   SystemPath path(0,0,0,0,1);
//   LuaObject<SystemPath>::PushToLua(path);
//
//   // Create an object, fully owned by lua
//   // WARNING! the lifetime of an object will be determined by the lua garbage
//   // collector, so a pointer to it should not be stored in any form on the C++ side
//   LuaObject<Ship>::CreateInLua(ship_id); // constructor arguments are passed
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

/*
 * SerializerPair stores object hooks for serializing and deserializing objects
 * into and out of json. The serialization mechanism functions in a type-erased
 * manner; the hook is responsible for loading the serialized object from the
 * Lua stack and in turn pushing the deserialized object on the stack as well.
 */
struct SerializerPair {
	// Serializer takes a lua object on the top of the stack and writes it to `out`
	using Serializer = bool (*)(lua_State *l, Json &out);
	// Deserializer takes `obj` and creates a lua object on the top of the stack
	using Deserializer = bool (*)(lua_State *l, const Json &obj);

	SerializerPair() :
		serialize(nullptr),
		deserialize(nullptr)
	{}

	SerializerPair(Serializer serialize_, Deserializer deserialize_) :
		serialize(serialize_),
		deserialize(deserialize_)
	{}

	Serializer serialize;
	Deserializer deserialize;
};

class PropertyMap;
class LuaMetaTypeBase;

// wrapper baseclass, and extra bits for getting at certain parts of the
// LuaObject layer
class LuaObjectBase {
	friend class LuaSerializer;

public:
	// creates a single "typeless" object and attaches the listed methods,
	// attributes and metamethods to it. leaves the created object on the
	// stack
	static void CreateObject(const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta, bool protect = false);

	// Creates a single "typeless" object and attaches the given metatype
	// to it. Leaves the created object on the stack.
	static void CreateObject(LuaMetaTypeBase *metaType);

	// Checks if the object at the stack position is a PropertiedObject and returns a pointer to
	// its PropertyMap. Returns nullptr on failure.
	static PropertyMap *GetPropertiesFromObject(lua_State *l, int object);

	// register a serializer pair for a given type
	static void RegisterSerializer(const char *type, SerializerPair pair);

	// Serialize all lua components registered for the given object.
	// Requires the LuaWrappable object to have been pushed to Lua at least once and still exist.
	// LuaComponents are only valid for LuaCoreObjects (in practice, only Body descendants)
	static bool SerializeComponents(LuaWrappable *object, Json &out);

	// Deserialize all lua components saved for the given object
	// Requires the LuaWrappable object to have been pushed to Lua at least once and still exist
	// LuaComponents are only valid for LuaCoreObjects (in practice, only Body descendants)
	static bool DeserializeComponents(LuaWrappable *object, const Json &obj);

	// Lookup the given LuaWrappable-derived object and deregister it if present.
	// Remove the object->wrapper from the registry. checks to make sure the
	// the mapping matches first, to protect against memory being reused
	// Transient (garbage-collected) LuaObjects should not be deregistered.
	static void DeregisterObject(LuaWrappable *object);

protected:
	// base class constructor, called by the wrapper Push* methods
	LuaObjectBase(const char *type) :
		m_type(type){};
	virtual ~LuaObjectBase() {}

	// creates a class in the lua vm with the given name and attaches the
	// listed methods to it and the listed metamethods to its metaclass. if
	// attributes extra magic is added to the metaclass to make them work as
	// expected
	static void CreateClass(const char *type, const char *parent, const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta);

	// Creates a class in the lua vm with the given name and attaches the
	// listed metatype to it. All method, attribute, and metamethod handling
	// is contained in the metatype.
	static void CreateClass(LuaMetaTypeBase *metaType);

	// Push an already-registered object onto the lua stack. the object is
	// looked up in the lua registry, if it exists its userdata is pushed
	// onto the lua stack. Returns true if the object exists and was pushed,
	// false otherwise
	static bool PushRegistered(LuaWrappable *o);

	// Adds an object->wrapper mapping to the registry for the given wrapper
	// object. The wrapper's corresponding userdata should be on the top of
	// the stack.
	// This method registers the given userdata in the transient registry and
	// the LuaObject will be garbage-collected if no reference is held in Lua.
	// This should only be used for ref-counted pointers or lua-owned objects.
	static void Register(LuaObjectBase *lo);

	// Adds an object->wrapper mapping to the persistent registry for the
	// given wrapper object (will never be garbage collected).
	// The wrapper's corresponding userdata should be on the top of the
	// stack.
	static void RegisterPersistent(LuaObjectBase *lo);

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

	// Take a lua object at the top of the stack and serialize it to Json
	static bool SerializeToJson(lua_State *l, Json &out);

	// Take a json object and deserialize it to a lua object
	static bool DeserializeFromJson(lua_State *l, const Json &obj);

	// allocate n bytes from Lua memory and leave it an associated userdata on
	// the stack. this is a wrapper around lua_newuserdata
	static void *Allocate(size_t n);

	// allocate uninitialized memory for an object of type T and leave it as an
	// associated userdata on the stack.
	template <typename T>
	static T *Allocate() { return static_cast<T *>(Allocate(sizeof(T))); }

	template <typename T, typename... Args>
	static T *AllocateNew(Args &&...args) { return new (Allocate<T>()) T(std::forward<Args>(args)...); }

	// get a pointer to the underlying object
	virtual LuaWrappable *GetObject() const = 0;

	// clear the underlying object pointer (turn this LuaObject into an "orphan" object)
	virtual void ClearObject() {}

	const char *GetType() const { return m_type; }

private:
	LuaObjectBase() {}
	LuaObjectBase(const LuaObjectBase &) {}

	// Lua-side helper functionality, declared in LuaObject.cpp
	friend class LuaObjectHelpers;

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
	static inline void PushToLua(DeleteEmitter *o);         // LuaCoreObject
	static inline void PushToLua(RefCounted *o);	        // LuaSharedObject
	static inline void PushToLua(const T &o);		        // LuaCopyObject
	static inline void PushComponentToLua(LuaWrappable *o); // LuaComponentObject
	template <typename... Args>
	static inline void CreateInLua(Args &&...args);

	template <typename Ret, typename Key, typename... Args>
	static inline Ret CallMethod(T *o, const Key &key, const Args &...args);
	template <typename Key, typename... Args>
	static inline void CallMethod(T *o, const Key &key, const Args &...args)
	{
		CallMethod<bool>(o, key, args...);
	}

	template <typename Ret1, typename Ret2, typename... Ret, typename Key, typename... Args>
	static inline std::tuple<Ret1, Ret2, Ret...> CallMethod(T *o, const Key &key, const Args &...args);

	// pull an object off the stack, unwrap and return it
	// if not found or doesn't match the type, throws a lua exception
	static inline T *CheckFromLua(int idx)
	{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
		return dynamic_cast<T *>(LuaObjectBase::CheckFromLua(idx, s_type));
#ifdef __clang__
#pragma clang diagnostic pop
#endif
	}

	// same but without error checks. returns 0 on failure
	static inline T *GetFromLua(int idx)
	{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
		return dynamic_cast<T *>(LuaObjectBase::GetFromLua(idx, s_type));
#ifdef __clang__
#pragma clang diagnostic pop
#endif
	}

	// standard cast promotion test for convenience
	static inline bool DynamicCastPromotionTest(LuaWrappable *o)
	{
		return dynamic_cast<T *>(o);
	}

protected:
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
	LuaObject() :
		LuaObjectBase(s_type)
	{}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

private:
	// initial lua type string. defined in a specialisation in the appropriate
	// .cpp file
	static const char *s_type;
};

// wrapper for a "core" object - one owned by c++ (eg Body).
// Lua needs to know when the object is deleted so that it can handle
// requests for it appropriately (ie with an exception, or exists())
// CoreObject pointers will be registered in the persistent registry and never
// garbage-collected while the referenced object is still alive.
template <typename T>
class LuaCoreObject : public LuaObject<T> {
public:
	LuaCoreObject(T *o) :
		m_object(o)
	{
		m_deleteConnection = m_object->DeleteEmitter::onDelete.connect(sigc::mem_fun(this, &LuaCoreObject::OnDelete));
	}

	~LuaCoreObject()
	{
		if (m_deleteConnection.connected())
			m_deleteConnection.disconnect();
	}

	LuaWrappable *GetObject() const override
	{
		return m_object;
	}

	void ClearObject() override
	{
		if (m_deleteConnection.connected())
			m_deleteConnection.disconnect();

		m_object = 0;
	}

private:
	void OnDelete()
	{
		LuaObjectBase::DeregisterObject(m_object);
		m_object = 0;
	}

	T *m_object;
	sigc::connection m_deleteConnection;
};

// Wrapper type for BodyComponent handles - deletion of components is handled
// in a callback from the BodyComponentDB. Component handles will be registered
// in the persistent registry and never garbage-collected while the component
// remains alive.
template<typename T>
class LuaComponentObject final : public LuaObject<T> {
public:
	LuaComponentObject(T *o) :
		m_object(o)
	{
	}

	LuaWrappable *GetObject() const override
	{
		return m_object;
	}

	void ClearObject() override
	{
		m_object = nullptr;
	}

private:
	T *m_object;
};

// wrapper for a "shared" object - one that can comfortably exist in both
// environments. usually for long-lived (StarSystem) or standalone (UI
// widget) objects
// Lua simply needs to keep a reference to these
template <typename T>
class LuaSharedObject : public LuaObject<T> {
public:
	LuaSharedObject(T *o) :
		m_object(o) {}

	LuaWrappable *GetObject() const override
	{
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
	LuaCopyObject(const T &o)
	{
		lua_State *l = Lua::manager->GetLuaState();
		m_object = LuaObjectBase::AllocateNew<T>(o);
		m_ref = LuaRef(l, -1);
		lua_pop(l, 1);
	}

	~LuaCopyObject()
	{
		m_object->~T();
		m_object = 0;
	}

	LuaWrappable *GetObject() const override
	{
		return m_object;
	}

private:
	T *m_object;
	LuaRef m_ref;
};

// wrapper for a "lua-owned" object.
// the wrapper is deleted by lua gc, and when it is deleted, it also deletes the wrapped object
template <typename T>
class LuaOwnObject : public LuaObject<T> {
public:
	LuaOwnObject(T *o)
	{
		m_object = o;
	}

	~LuaOwnObject()
	{
		delete (m_object);
	}

	LuaWrappable *GetObject() const override
	{
		return m_object;
	}

private:
	T *m_object;
};

// push methods, create wrappers if necessary
// wrappers are allocated from Lua memory
template <typename T>
inline void LuaObject<T>::PushToLua(DeleteEmitter *o)
{
	if (!PushRegistered(o))
		RegisterPersistent(AllocateNew<LuaCoreObject<T>>(static_cast<T *>(o)));
}

template<typename T>
inline void LuaObject<T>::PushComponentToLua(LuaWrappable *o)
{
	if (!PushRegistered(o))
		RegisterPersistent(AllocateNew<LuaComponentObject<T>>(static_cast<T *>(o)));
}

template <typename T>
inline void LuaObject<T>::PushToLua(RefCounted *o)
{
	if (!PushRegistered(o))
		Register(AllocateNew<LuaSharedObject<T>>(static_cast<T *>(o)));
}

template <typename T>
inline void LuaObject<T>::PushToLua(const T &o)
{
	Register(AllocateNew<LuaCopyObject<T>>(o));
}

template <typename T>
template <typename... Args>
inline void LuaObject<T>::CreateInLua(Args &&...args)
{
	T *p(new T(std::forward<Args>(args)...));
	Register(AllocateNew<LuaOwnObject<T>>(static_cast<T *>(p)));
}

template <typename T>
template <typename Ret, typename Key, typename... Args>
inline Ret LuaObject<T>::CallMethod(T *o, const Key &key, const Args &...args)
{
	lua_State *l = Lua::manager->GetLuaState();
	LUA_DEBUG_START(l);
	Ret return_value;

	lua_checkstack(l, sizeof...(args) + 5);
	PushToLua(o);
	pi_lua_generic_push(l, key);
	lua_gettable(l, -2);
	lua_pushvalue(l, -2);
	lua_remove(l, -3);
	pi_lua_multiple_push(l, args...);
	pi_lua_protected_call(l, sizeof...(args) + 1, 1);
	pi_lua_generic_pull(l, -1, return_value);
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
	return return_value;
}

template <typename T>
template <typename Ret1, typename Ret2, typename... Ret, typename Key, typename... Args>
inline std::tuple<Ret1, Ret2, Ret...> LuaObject<T>::CallMethod(T *o, const Key &key, const Args &...args)
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);
	lua_checkstack(l, sizeof...(args) + 5);
	PushToLua(o);
	pi_lua_generic_push(l, key);
	lua_gettable(l, -2);
	lua_pushvalue(l, -2);
	lua_remove(l, -3);
	pi_lua_multiple_push(l, args...);
	pi_lua_protected_call(l, sizeof...(args) + 1, 2 + sizeof...(Ret));
	auto ret_values = pi_lua_multiple_pull<Ret1, Ret2, Ret...>(l, -2 - static_cast<int>(sizeof...(Ret)));
	lua_pop(l, 2 + static_cast<int>(sizeof...(Ret)));
	LUA_DEBUG_END(l, 0);
	return ret_values;
}

// specialise for SystemPath, which needs custom machinery to deduplicate system paths
class SystemPath;
template <>
void LuaObject<SystemPath>::PushToLua(const SystemPath &o);

inline void pi_lua_generic_pull(lua_State *l, int index, SystemPath &out)
{
	assert(l == Lua::manager->GetLuaState());
	out = *LuaObject<SystemPath>::CheckFromLua(index);
}

inline void pi_lua_generic_push(lua_State *l, const SystemPath &value)
{
	assert(l == Lua::manager->GetLuaState());
	LuaObject<SystemPath>::PushToLua(&value);
}

// LuaPushPull stuff.
template <class T>
void pi_lua_generic_pull(lua_State *l, int index, T *&out)
{
	assert(l == Lua::manager->GetLuaState());
	out = LuaObject<typename std::remove_cv<T>::type>::CheckFromLua(index);
}

template <class T>
bool pi_lua_strict_pull(lua_State *l, int index, T *&out)
{
	assert(l == Lua::manager->GetLuaState());
	out = LuaObject<typename std::remove_cv<T>::type>::GetFromLua(index);
	return out != 0;
}

template <class T>
void pi_lua_generic_push(lua_State *l, T *value)
{
	assert(l == Lua::manager->GetLuaState());
	if (value)
		LuaObject<typename std::remove_cv<T>::type>::PushToLua(value);
	else
		lua_pushnil(l);
}

// LuaPushPull stuff.
template <class T>
void pi_lua_generic_pull(lua_State *l, int index, RefCountedPtr<T> &out)
{
	assert(l == Lua::manager->GetLuaState());
	out = LuaObject<typename std::remove_cv<T>::type>::CheckFromLua(index);
}

template <class T>
bool pi_lua_strict_pull(lua_State *l, int index, RefCountedPtr<T> &out)
{
	assert(l == Lua::manager->GetLuaState());
	out = LuaObject<typename std::remove_cv<T>::type>::GetFromLua(index);
	return out != 0;
}

template <class T>
void pi_lua_generic_push(lua_State *l, RefCountedPtr<T> value)
{
	assert(l == Lua::manager->GetLuaState());
	if (value)
		LuaObject<typename std::remove_cv<T>::type>::PushToLua(value.Get());
	else
		lua_pushnil(l);
}

#endif
