#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>

#include "LuaManager.h"
#include "DeleteEmitter.h"

//
// LuaObject provides proxy objects and tracking facilities to safely get
// objects in and out of lua. the basic idea is that for every class you want
// to expose to lua, you define LuaObject wrapper class that defines the
// lua name lua name, methods and metamethods for that class. you then call
// methods on this class to push and pull objects to and from the lua stack
//
//
// if you just want to use it, do something like:
//
// push a value onto the lua stack before method call or return (you (c++)
// remains responsible for deallocating the object)
//
//   Ship *s = new Ship("Eagle Long Range Fighter");
//   LuaShip::PushToLua(s);
//
// push a value onto the lua stack (lua will deallocate the object when it
// goes out of scope and the garbage collector runs. OBJECT MUST BE
// HEAP-ALLOCATED)
//
//   Ship *s = new Ship("Eagle Long Range Fighter");
//   LuaShip::PushToLuaGC(s);
//
// get a value from the lua stack at index n (causes lua exception if the
// object doesn't exist or the types don't match)
//
//   Ship *s = LuaShip::GetFromLua(1);
//
// note that it uses the singleton lua state provided by LuaManager. there's
// no facility to use a different lua state. if you think you need to you're
// probably doing something wrong. ask the devs if you're not sure.
//
//
// if you need to expose a new class to lua, do this (using Ship/LuaShip as an
// example)
//
//  - have your class inherit from DeleteEmitter
//
//  - add a typedef for the wrapper class to the bottom of LuaObjectBase.h:
//
//      class Ship;
//      typedef LuaObject<Ship> LuaShip;
//
//  OR if your class can't inherit from DeleteEmitter for some reason, use
//  this kind of typedef instead to create a subclass
//
//      class SBodyPath;
//      typedef LuaObjectUncopyable<SBodyPath,LuaUncopyable<SBodyPath> > LuaSBodyPath;
//
//  you probably don't want to do this unless you understand the entire
//  LuaObjectBase system. read on and ask for help :)
//
//  - arrange for the wrapper class RegisterClass() method to be called in
//    LuaManager::Init in LuaManager.cpp
//
//      LuaShip::RegisterClass();
//
//  - make a new file LuaShip.cpp with static data for the lua class name and
//  method/metamethod pointer tables (copy from one of the others to get the
//  idea)
//
//  - add the new file to the build system
//
//  - implement your lua methods in that file
// 
//
// if you want to understand how this works, read on :)
//
// the basic premise of all this is that lua never holds a pointer to the c++
// object. instead, when a c++ object is pushed to the lua stack a lua object
// (that is, a value with a metatable) is created that internally has a
// numeric identifier. the original object is added to a hashtable (the
// registry) with the identifier as the key. when lua calls a method against
// the object, the appropriate method/metamethod is called with the lua object
// on the stack. the method implementation (c++) then pulls these objects from
// the stack via wrapper classes that handle the details of looking up the
// real object in the registry
//
// objects are removed from the registry when either the c++ or the lua side
// releases them. on the lua side this is handled by the __gc metamethod. on
// the c++ side this is done via the DeleteEmitter class that all wrapped
// classes are expected to inherit from. a callback is add to the onDelete
// signal when the object is registered. when either of these things are
// called the object is removed from the registry. the callback will also
// delete the object if it was pushed with PushToLuaGC to indicate that lua
// owns the object
//

// type for internal object identifiers
typedef uintptr_t lid;

// the base class for wrapper classes. the only public interface is for
// lookup. creation is handled by the wrapper classes
class LuaObjectBase {
public:
	// pull an LuaObjectBase wrapper from the registry given an id. returns NULL
	// if the object is not in the registry
	static LuaObjectBase *Lookup(lid id);

	// accessors for the wrapper data. only really used by the serializer
	inline lid GetId() const { return m_id; }
	inline DeleteEmitter *GetObject() const { return m_object; }
	inline const char *GetType() const { return m_type; }

    // determine if the object has a class in its ancestry
    bool Isa(const char *want_type) const;

protected:
	virtual ~LuaObjectBase() {}

	// instantiate an object wrapper. it gets added to the registry then a new
	// lua object is created for it and pushed onto the lua stack
	// type is the lua type string. wantdelete indicates if LuaObjectBase should
	// delete the c++ object when it is removed from the registry
	LuaObjectBase(DeleteEmitter *o, const char *type, bool wantdelete);

	// creates a class in the lua vm with the given name and attaches the
	// listed methods to it and the listed metamethods to its metaclass
	static void CreateClass(const char *type, const char *inherit, const luaL_reg methods[], const luaL_reg meta[]);

	// pulls an object off the lua stack and returns its associated c++
	// object. want_type is the lua type string of the object. a lua exception
	// is triggered if the object on the stack is not of this type
	static DeleteEmitter *GetFromLua(int index, const char *want_type);


private:
	LuaObjectBase(const LuaObjectBase &) {}

	// remove an object from the registry. deletes lo and the underlying c++
	// object if necessary
	static void Deregister(LuaObjectBase *lo);

	// the lua object "destructor" that gets called by the garbage collector.
	// its only part of the class so that it can call Deregister()
	static int GC(lua_State *l);

	// object id, pointer to the c++ object and lua type string
	lid            m_id;
	DeleteEmitter *m_object;
	const char    *m_type;

	// flag to indicate that lua owns the object and should delete it when its
	// deregistered
	bool m_wantDelete;

	// the wrapper object's connection to the deletion signal. this gets
	// connected on registration and disconnected on deregistration
	sigc::connection m_deleteConnection;
};


// template for a wrapper class
template <typename T>
class LuaObject : public LuaObjectBase {
public:

	// registers the class with the lua vm
	static inline void RegisterClass() {
		CreateClass(s_type, s_inherit, s_methods, s_meta);
	};

	// wrap the object and push it onto the lua stack
	static inline void PushToLua(T *o) {
		new LuaObject(o, false);
	}

	// wrap the object and push it onto the lua stack, taking ownership of it
	static inline void PushToLuaGC(T *o) {
		new LuaObject(o, true);
	}

	// pull an object off the the stack, unwrap it and return it
	static inline T *GetFromLua(int index) {
		return dynamic_cast<T *>(LuaObjectBase::GetFromLua(index, s_type));
	}

private:
	LuaObject(T *o, bool wantdelete) : LuaObjectBase(o, s_type, wantdelete) {}

	// lua type string, optional parent type, method table and metamethod
	// table. these are defined per wrapper class in the appropriate .cpp file
	static const char *s_type;
	static const char *s_inherit;
	static const luaL_reg s_methods[];
	static const luaL_reg s_meta[];
};


// this one is more complicated. if a class needs to be copyable it can't
// inherit from DeleteEmitter as required to be wrapped by LuaObject. so we
// create a new class and inherit from both. it takes a full copy of the
// original when instantiated, so is decoupled from the original
template <typename T>
class LuaUncopyable : public T, public DeleteEmitter {
public:
	LuaUncopyable(const T &p) : T(p), DeleteEmitter() {}
private:
	LuaUncopyable() {}
	LuaUncopyable(const LuaUncopyable &) {}
};

// if we wanted we could just use LuaUncopyable<T> as-is, but that would mean
// having to create a uncopyable of every object before passing it to
// PushToLua() and always casting the return from GetFromLua(). instead we
// subclass the object and implement some wrapper methods for the "real"
// types.
template <typename T, typename UT>
class LuaObjectUncopyable : LuaObject<UT> {
public:
	static inline void RegisterClass() { LuaObject<UT>::RegisterClass(); }

	// create an uncopyable version and pass it in. we use PushToLuaGC because
	// this is our object and we have to clean it up
	static inline void PushToLua(T *p) {
		UT *up = new UT(*p);
		LuaObject<UT>::PushToLuaGC(up);
	}

	// same idea, but caller asked us to clean it up when we're done so we
	// have to do that straight away
	static inline void PushToLuaGC(T *p) {
		UT *up = new UT(*p);
		delete p;
		LuaObject<UT>::PushToLuaGC(up);
	}

	// pull from lua, casting back to the original type
	static inline T *GetFromLua(int index) {
		return dynamic_cast<T*>(LuaObject<UT>::GetFromLua(index));
	}
};


// these are push/pull functions to provide a consistent interface for
// primitive types
namespace LuaString {
	inline void PushToLua(const char *s) {
		assert(s);
		lua_State *l = LuaManager::Instance()->GetLuaState();
		lua_pushstring(l, s);
	}
	inline std::string GetFromLua(int index) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		std::string s = luaL_checkstring(l, index);
		return s;
	}
};

namespace LuaFloat {
	inline void PushToLua(double n) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		lua_pushnumber(l, n);
	}
	inline double GetFromLua(int index) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		double n = luaL_checknumber(l, index);
		return n;
	}
};

namespace LuaInt {
	inline void PushToLua(int n) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		lua_pushinteger(l, n);
	}
	inline int GetFromLua(int index) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		int n = luaL_checkinteger(l, index);
		return n;
	}
};

namespace LuaBool {
	inline void PushToLua(bool b) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		lua_pushboolean(l, b);
	}
	inline int GetFromLua(int index) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
        if (!lua_isboolean(l, index))
            luaL_typerror(l, index, lua_typename(l, LUA_TBOOLEAN));
        bool b = lua_toboolean(l, index);
		return b;
	}
};

// define types for the interesting classes
class Body;
typedef LuaObject<Body> LuaBody;

class Ship;
typedef LuaObject<Ship> LuaShip;

class SpaceStation;
typedef LuaObject<SpaceStation> LuaSpaceStation;

class Planet;
typedef LuaObject<Planet> LuaPlanet;

class Star;
typedef LuaObject<Star> LuaStar;

class Player;
typedef LuaObject<Player> LuaPlayer;

class StarSystem;
typedef LuaObject<StarSystem> LuaStarSystem;

class SBodyPath;
typedef LuaObjectUncopyable<SBodyPath,LuaUncopyable<SBodyPath> > LuaSBodyPath;

#endif
