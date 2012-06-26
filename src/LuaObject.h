#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

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
//      class SystemPath;
//      typedef LuaObjectUncopyable<SystemPath,LuaUncopyable<SystemPath> > LuaSystemPath;
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
// a object pointer (lightuserdata) -> userdata mapping is held in the lua
// registry. when a value is pushed this is consulted first. if lua already
// knows about the object the userdata is reused. this ensures that objects
// that only exist once outside lua also only exist once inside lua, and means
// that object equality works as expected. these mapping holds weak references
// so that the operation of the garbage collector is not affected
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
typedef unsigned int lid;

// type for promotion test callbacks
typedef bool (*PromotionTest)(DeleteEmitter *o);

// the base class for wrapper classes. it has no public interface. everything
// you need goes through the wrapper classes
class LuaObjectBase {
	friend class LuaSerializer;

public:
	// creates a single "typeless" object and attaches the listed methods,
	// attributes and metamethods to it. leaves the created object on the
	// stack
	static void CreateObject(const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta);

protected:
	// base class constructor, called by the wrapper Push* methods
	LuaObjectBase(DeleteEmitter *o, const char *type) : m_object(o), m_type(type) {};
	virtual ~LuaObjectBase() {}

	// creates a class in the lua vm with the given name and attaches the
	// listed methods to it and the listed metamethods to its metaclass. if
	// attributes extra magic is added to the metaclass to make them work as
	// expected
	static void CreateClass(const char *type, const char *parent, const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta);

	// push an already-registered object onto the lua stack. the object is
	// looked up in the lua registry, if it exists a copy of its userdata is
	// placed on the lua stack. returns true if the object exist and was
	// pushed, false otherwise
	static bool PushRegistered(DeleteEmitter *o);

	// pushes the raw object into lua. new userdata is create and stored in
	// the lookup table in the lua registry for PushRegistered to user later,
	// and then is added to the stack. if wantdelete is true, lua takes
	// control of the object and will call delete on it when it is finished
	// with it
	static void Push(LuaObjectBase *lo, bool wantdelete);

	// pulls an object off the lua stack and returns its associated c++
	// object. type is the lua type string of the object. a lua exception is
	// triggered if the object on the stack is not of this type
	static DeleteEmitter *GetFromLua(int index, const char *type);

	// does exactly the same as GetFromLua without triggering exceptions
	static DeleteEmitter *CheckFromLua(int index, const char *type);

	// register a promotion test. when an object with lua type base_type is
	// pushed, test_fn will be called. if it returns true then the created lua
	// object will be of target_type
	static void RegisterPromotion(const char *base_type, const char *target_type, PromotionTest test_fn);

	// abstract functions for the object acquire/release functions. these are
	// called to somehow record that the object is "in use". the wrapper class
	// handles the hard details of this (most of the time it results in a noop)
	virtual void Acquire(DeleteEmitter *) = 0;
	virtual void Release(DeleteEmitter *) = 0;

private:
	LuaObjectBase() {}
	LuaObjectBase(const LuaObjectBase &) {}

	// remove an object from the registry. deletes lo and the underlying c++
	// object if necessary
	static void Deregister(LuaObjectBase *lo);

	// lua method to determine if the underlying object is still present in
	// the registry (ie still exists)
	static int l_exists(lua_State *l);

	// lua method to determine if the object inherits from a type. wrapper
	// around ::Isa()
	static int l_isa(lua_State *l);

	// the lua object "destructor" that gets called by the garbage collector.
	// its only part of the class so that it can call Deregister()
	static int l_gc(lua_State *l);

	// default tostring. shows a little more info about the object, like its
	// type
	static int l_tostring(lua_State *l);

	// pull an LuaObjectBase wrapper from the registry given an id. returns NULL
	// if the object is not in the registry
	static LuaObjectBase *Lookup(lid id);

    // determine if the object has a class in its ancestry
    bool Isa(const char *base) const;

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


// basic acquirer template. used by the wrapper to implement
// LuaObjectBase::Acquire and LuaObjectBase::Release. this is the general
// case, which does nothing at all
template <typename T>
class LuaAcquirer {
public:
	virtual void OnAcquire(T *) {}
	virtual void OnRelease(T *) {}
};

// acquirer baseclass for RefCounted types. subclass this when you need Lua to
// take a reference to an object
class LuaAcquirerRefCounted {
public:
	virtual void OnAcquire(RefCounted *o) {
		o->IncRefCount();
	}
	virtual void OnRelease(RefCounted *o) {
		o->DecRefCount();
	}
};

// template for a wrapper class
template <typename T>
class LuaObject : public LuaObjectBase, LuaAcquirer<T> {
public:

	// registers the class with the lua vm
	static void RegisterClass();

	// wrap the object and push it onto the lua stack
	static inline void PushToLua(T *o) {
		if (! LuaObjectBase::PushRegistered(o))
			LuaObjectBase::Push(new LuaObject(o), false);
	}

	// wrap the object and push it onto the lua stack, taking ownership of it
	static inline void PushToLuaGC(T *o) {
		if (! LuaObjectBase::PushRegistered(o))
			LuaObjectBase::Push(new LuaObject(o), true);
	}

	// pull an object off the the stack, unwrap it and return it
	static inline T *GetFromLua(int index) {
		return dynamic_cast<T *>(LuaObjectBase::GetFromLua(index, s_type));
	}

	static inline T *CheckFromLua(int index) {
		return dynamic_cast<T *>(LuaObjectBase::CheckFromLua(index, s_type));
	}

	// convenience promotion test
	static inline bool DynamicCastPromotionTest(DeleteEmitter *o) {
		return dynamic_cast<T *>(o);
	}

protected:
	// hook up the appropriate acquirer for the wrapped object.
	virtual void Acquire(DeleteEmitter *o) { this->LuaAcquirer<T>::OnAcquire(dynamic_cast<T*>(o)); }
	virtual void Release(DeleteEmitter *o) { this->LuaAcquirer<T>::OnRelease(dynamic_cast<T*>(o)); }

private:
	LuaObject(T *o) : LuaObjectBase(o, s_type) {}

	// lua type string. this is defined per wrapper class in the appropriate
    // .cpp file
	static const char *s_type;
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

	static inline T *CheckFromLua(int index) {
		return dynamic_cast<T*>(LuaObject<UT>::CheckFromLua(index));
	}

private:
	LuaObjectUncopyable() {}
};

#endif
