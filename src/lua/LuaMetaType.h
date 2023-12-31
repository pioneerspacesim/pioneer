// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Lua.h"
#include "LuaCall.h"
#include "LuaManager.h"
#include "LuaPushPull.h"
#include "LuaTable.h"

/*
 * LuaMetaType is a C++ -> Lua binding system intended to drastically
 * simplify the amount of time and code needed to write often-repetitive
 * glue code to pull arguments from Lua and pass them to a C++ function,
 * then push the result back on the Lua stack.
 *
 * Usage comes in two flavors based on what you're intending to bind:
 *
 * LuaObjects (e.g. classes and structs):
 *
 *   In the regular LuaObject<T>::RegisterClass method, create a static
 *   LuaMetaType<T> object and call CreateMetaType(lua_State *) on it.
 *   Once done, between any StartRecording() and StopRecording() pair,
 *   chain AddFunction and AddMember calls to build the Lua binding like so:
 *
 *   > .AddFunction("name", &T::SomeFunc)
 *   > .AddMember("name", &T::someMember)
 *
 *   Call LuaObjectBase::CreateClass(&metaType) afterwards and you're done.
 *
 * Generic "Namespace" bindings (e.g. LuaEngine):
 *
 *    These are similar to the above, simply create a LuaMetaTypeGeneric
 *    object. Instead of binding c++ member functions however, you can
 *    bind any C++ free function - LuaMetaTypeGeneric doesn't have a concept
 *    of a 'this object' and is intended to simplify binding libraries of
 *    stateless functions to Lua (e.g. ImGui).
 *
 *    > .AddFunction("NewLine", &ImGui::NewLine)
 */

// Base class for the LuaMetaType binding system
class LuaMetaTypeBase {
public:
	LuaMetaTypeBase(const char *name) :
		m_typeName(name)
	{}

	// Creates and registers the lua-side object for this type.
	void CreateMetaType(lua_State *l);

	const char *GetTypeName() const { return m_typeName.c_str(); }

	const char *GetParent() const { return m_parent.c_str(); }

	void SetParent(const char *parent) { m_parent = parent; }

	// Push the metatable to the lua stack.
	void GetMetatable() const;

	// Returns true if the metatype has been correctly set up.
	bool IsValid() const { return m_lua && m_ref != LUA_NOREF; }

	// Call this function to set the lua stack up to begin recording members
	// and methods into the metatype.
	// It is invalid to call other functions outside a StartRecording / StopRecording pair.
	void StartRecording()
	{
		assert(IsValid());
		assert(m_index == 0);

		GetMetatable();
		m_index = lua_gettop(m_lua);
	}

	// Stop recording and remove the metatype from the stack
	void StopRecording()
	{
		assert(IsValid());
		assert(m_index != 0);
		lua_remove(m_lua, m_index);
		m_index = 0;
	}

	// Get all valid method/attribute names for the object on the top of the stack.
	// Mainly intended to be used by the console, though it can also be used for
	// debug dumping of properties, attributes, and methods
	static void GetNames(std::vector<std::string> &names, const std::string &prefix = "", bool methodsOnly = false);

	// Get the lua-side metatable from a type name instead of a LuaRef.
	static bool GetMetatableFromName(lua_State *l, const char *name);

	// A replacement for luaL_testudata that is metatype-aware
	static void *TestUserdata(lua_State *l, int index, const char *name);

	// A replacement for luaL_checkudata that is metatype-aware
	static void *CheckUserdata(lua_State *l, int index, const char *name);

protected:
	//=========================================================================
	// LuaMetaTypeGeneric support:
	//=========================================================================

	// C++ free functions taking any set of arguments and returning a value or void
	template <typename Rt, typename... Args>
	using free_function = Rt (*)(Args...);

	// Overload for free functions returning a value
	template <typename Rt, typename... Args>
	typename std::enable_if<!std::is_same<Rt, void>::value, int>::type static free_fn_wrapper_(lua_State *L)
	{
		if (size_t(lua_gettop(L)) < sizeof...(Args))
			return luaL_error(L, "Invalid number of arguments for function %s (have %d, need %lu)",
				lua_tostring(L, lua_upvalueindex(1)), lua_gettop(L), sizeof...(Args));

		auto fn = PullFreeFunction<free_function<Rt, Args...>>(L, lua_upvalueindex(2));
		Rt ret = pi_lua_multiple_call(L, 1, fn);
		LuaPush<Rt>(L, ret);

		return 1;
	}

	// Overload for functions returning void
	template <typename Rt, typename... Args>
	typename std::enable_if<std::is_same<Rt, void>::value, int>::type static free_fn_wrapper_(lua_State *L)
	{
		if (size_t(lua_gettop(L)) < sizeof...(Args))
			return luaL_error(L, "Invalid number of arguments for function %s (have %d, need %lu)",
				lua_tostring(L, lua_upvalueindex(1)), lua_gettop(L), sizeof...(Args));

		auto fn = PullFreeFunction<free_function<Rt, Args...>>(L, lua_upvalueindex(2));
		pi_lua_multiple_call(L, 1, fn);

		return 0;
	}

	//=========================================================================
	// LuaMetaType<T> support:
	//=========================================================================

	// Direct member pointer access (for structs, etc.)
	template <typename T, typename Dt>
	using member_pointer = Dt T::*;

	template <typename T, typename Dt>
	static int member_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));

		if (!ptr)
			return luaL_error(L, "Null or invalid userdata accessed for property %s", name);

		if (lua_gettop(L) > 2)
			return luaL_error(L, "Invalid number of arguments for property getter/setter %s", name);

		auto &t = PullPointerToMember<member_pointer<T, Dt>>(L, lua_upvalueindex(2));
		if (lua_gettop(L) == 1) {
			LuaPush<Dt>(L, ptr->*(t));
			return 1;
		} else {
			(ptr->*(t)) = LuaPull<Dt>(L, 2);
			return 0;
		}
	}

	//=========================================================================

	// "Member Free Function" access: like lua_CFunction but with an automatic
	// this-object pointer passed to the function for simplicity
	template <typename T>
	using member_cfunction = int (*)(lua_State *, T *);

	template <typename T>
	static int member_cfn_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));
		luaL_checktype(L, lua_upvalueindex(2), LUA_TLIGHTUSERDATA);

		if (!ptr)
			return luaL_error(L, "Invalid userdata accessed for function %s", name);

		auto fn = PullFreeFunction<member_cfunction<T>>(L, lua_upvalueindex(2));
		return fn(L, ptr);
	}

	// Bind two member free functions into a getter-setter pair for an attr
	template <typename T>
	static int getter_member_cfn_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));
		auto getter = PullFreeFunction<member_cfunction<T>>(L, lua_upvalueindex(2));

		if (!ptr)
			return luaL_error(L, "Null or invalid userdata accessed for property %s", name);

		if (lua_gettop(L) > 2)
			return luaL_error(L, "Invalid number of arguments for property getter/setter %s", name);

		if (lua_gettop(L) > 1) {
			auto setter = PullFreeFunction<member_cfunction<T>>(L, lua_upvalueindex(3));
			if (setter != nullptr)
				return setter(L, ptr);
			else
				return luaL_error(L, "Attempt to call undefined setter for property %s", name);
		}

		return getter(L, ptr);
	}

	//=========================================================================

	// C++ member function pointer access: using template magic, this assembles
	// all of the bridge code needed to pull values from Lua and pass the return value back
	template <typename T, typename Rt, typename... Args>
	using member_function = Rt (T::*)(Args...);

	template <typename T, typename Rt, typename... Args>
	using const_member_function = Rt (T::*)(Args...) const;

	// Overload for function returning a value
	template <typename T, typename Rt, typename... Args>
	typename std::enable_if<!std::is_same<Rt, void>::value, int>::type static member_fn_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));

		if (!ptr)
			return luaL_error(L, "Invalid userdata accessed for function %s", name);

		if (size_t(lua_gettop(L) - 1) < sizeof...(Args))
			return luaL_error(L, "Invalid number of arguments for function %s", name);

		auto &fn = PullPointerToMember<member_function<T, Rt, Args...>>(L, lua_upvalueindex(2));
		Rt ret = pi_lua_multiple_call(L, 1, ptr, fn);
		LuaPush<Rt>(L, ret);

		return 1;
	}

	// Overload for function returning void
	template <typename T, typename Rt, typename... Args>
	typename std::enable_if<std::is_same<Rt, void>::value, int>::type static member_fn_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));

		if (!ptr)
			return luaL_error(L, "Invalid userdata accessed for function %s", name);

		if (size_t(lua_gettop(L) - 1) < sizeof...(Args))
			return luaL_error(L, "Invalid number of arguments for function %s", name);

		auto &fn = PullPointerToMember<member_function<T, Rt, Args...>>(L, lua_upvalueindex(2));
		pi_lua_multiple_call(L, 1, ptr, fn);

		return 0;
	}

	// T::GetName / T::SetName glue code to wrap into a single attr function
	template <typename T, typename Dt, typename Dt2>
	static int getter_member_fn_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));
		auto &getter = PullPointerToMember<member_function<T, Dt>>(L, lua_upvalueindex(2));

		if (!ptr)
			return luaL_error(L, "Null or invalid userdata accessed for property %s", name);

		if (lua_gettop(L) > 2)
			return luaL_error(L, "Invalid number of arguments for property getter/setter %s", name);

		if (lua_gettop(L) > 1) {
			auto &setter = PullPointerToMember<member_function<T, void, Dt2>>(L, lua_upvalueindex(3));
			if (setter != nullptr) {
				pi_lua_multiple_call(L, 1, ptr, setter);
				return 0;
			} else
				return luaL_error(L, "Attempt to call undefined setter for property %s", name);
		} else {
			Dt value = pi_lua_multiple_call(L, 1, ptr, getter);
			LuaPush<Dt>(L, value);
			return 1;
		}
	}

	//=========================================================================
	// Generic support for getting function pointers in and out of lua:
	//=========================================================================

	template <typename MemT>
	static void PushPointerToMember(lua_State *L, MemT obj)
	{
		*reinterpret_cast<MemT *>(lua_newuserdata(L, sizeof(MemT))) = obj;
	}

	template <typename MemT>
	static MemT &PullPointerToMember(lua_State *L, int idx)
	{
		return *reinterpret_cast<MemT *>(lua_touserdata(L, idx));
	}

	template <typename T>
	static void PushFreeFunction(lua_State *L, T obj)
	{
		static_assert(sizeof(T) == sizeof(void *), "Free functions cannot be 'fat' lambdas!");
		lua_pushlightuserdata(L, reinterpret_cast<void *>(obj));
	}

	template <typename T>
	static T PullFreeFunction(lua_State *L, int index)
	{
		return reinterpret_cast<T>(lua_touserdata(L, index));
	}

	// Pushes a copy of the metatable's attribute table to the stack
	void GetAttrTable(lua_State *L, int index)
	{
		luaL_getsubtable(L, index, "attrs");
	}

	// Pushes a copy of the metatable's attribute table to the stack
	void GetMethodTable(lua_State *L, int index)
	{
		luaL_getsubtable(L, index, "methods");
	}

	std::string m_typeName;
	std::string m_parent;

	lua_State *m_lua;
	// the reference id of the metatable
	int m_ref = LUA_NOREF;
	// the position of the metatable on the stack while recording
	int m_index = 0;
};

class LuaMetaTypeGeneric : public LuaMetaTypeBase {
public:
	using Self = LuaMetaTypeGeneric;

	LuaMetaTypeGeneric(const char *name) :
		LuaMetaTypeBase(name)
	{}

	Self &StartRecording()
	{
		LuaMetaTypeBase::StartRecording();
		return *this;
	}

	Self &StopRecording()
	{
		LuaMetaTypeBase::StopRecording();
		return *this;
	}

	Self &SetProtected(bool enabled)
	{
		m_protected = enabled;
		return *this;
	}

	Self &AddMember(const char *name, lua_CFunction getter)
	{
		GetAttrTable(m_lua, m_index);

		lua_pushcfunction(m_lua, getter);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, -2, name);
		lua_pop(m_lua, 1);
		return *this;
	}

	Self &AddFunction(const char *name, lua_CFunction func)
	{
		GetMethodTable(m_lua, m_index);

		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, -2, name);
		lua_pop(m_lua, 1);
		return *this;
	}

	template <typename Rt, typename... Args>
	Self &AddFunction(const char *name, free_function<Rt, Args...> func)
	{
		GetMethodTable(m_lua, m_index);

		lua_pushstring(m_lua, (m_typeName + "." + name).c_str());
		PushFreeFunction(m_lua, func);
		lua_pushcclosure(m_lua, &free_fn_wrapper_<Rt, Args...>, 2);
		if (m_protected)
			lua_pushcclosure(m_lua, &secure_trampoline, 1);

		lua_setfield(m_lua, -2, name);
		lua_pop(m_lua, 1);
		return *this;
	}

	Self &AddMeta(const char *name, lua_CFunction func)
	{
		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, m_index, name);

		return *this;
	}

	template <typename Rt, typename... Args>
	Self &AddMeta(const char *name, free_function<Rt, Args...> func)
	{
		lua_pushstring(m_lua, (m_typeName + "." + name).c_str());
		PushFreeFunction(m_lua, func);
		lua_pushcclosure(m_lua, &free_fn_wrapper_<Rt, Args...>, 2);
		if (m_protected)
			lua_pushcclosure(m_lua, &secure_trampoline, 1);

		lua_setfield(m_lua, m_index, name);

		return *this;
	}

	Self &AddCallCtor(lua_CFunction func)
	{
		GetMethodTable(m_lua, m_index);
		lua_getmetatable(m_lua, -1);

		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, -2, "__call");
		lua_pop(m_lua, 2);

		return *this;
	}

	Self &AddNewCtor(lua_CFunction func)
	{
		GetMethodTable(m_lua, m_index);

		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, -2, "New");
		lua_pop(m_lua, 1);
		return *this;
	}

private:
	bool m_protected = false;
};

template <typename T>
class LuaMetaType : public LuaMetaTypeBase {
public:
	using Self = LuaMetaType;

	LuaMetaType(const char *name) :
		LuaMetaTypeBase(name)
	{}

	Self &StartRecording()
	{
		LuaMetaTypeBase::StartRecording();
		return *this;
	}

	Self &StopRecording()
	{
		LuaMetaTypeBase::StopRecording();
		return *this;
	}

	// All functions and members pushed while protection is enabled will error
	// when accessed by a non-trusted lua script.
	void SetProtected(bool enabled) { m_protected = enabled; }

	// Bind a raw C++ data member to Lua.
	// Obviously, the member in question must be publically accessible, or
	// LuaMetaTypeBase must be marked as a friend class.
	template <typename Dt>
	Self &AddMember(const char *name, member_pointer<T, Dt> t)
	{
		lua_State *L = m_lua;
		GetAttrTable(L, m_index);

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushPointerToMember(L, t);
		lua_pushcclosure(L, &member_wrapper_<T, Dt>, 2);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, -2, name);
		lua_pop(L, 1);

		return *this;
	}

	// Bind a pseudo-member to Lua via a free-function getter and setter.
	// The getter and setter are responsible for pulling parameters from Lua.
	Self &AddMember(const char *name, member_cfunction<T> getter, member_cfunction<T> setter = nullptr)
	{
		lua_State *L = m_lua;
		GetAttrTable(L, m_index);

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushFreeFunction(L, getter);
		PushFreeFunction(L, setter);
		lua_pushcclosure(L, &getter_member_cfn_wrapper_<T>, 3);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, -2, name);
		lua_pop(L, 1);

		return *this;
	}

	// Magic to allow binding a const function to Lua. Take care to ensure that you do not
	// push a const object to lua, or this code will become undefined behavior.
	template <typename Dt, typename Dt2 = Dt>
	Self &AddMember(const char *name, const_member_function<T, Dt> getter, member_function<T, void, Dt2> setter = nullptr)
	{
		return AddMember(name, reinterpret_cast<member_function<T, Dt>>(getter), setter);
	}

	// Bind a pseudo-member to Lua via a member-function getter and setter.
	// The parameter will automatically be pulled from Lua and passed to the setter.
	template <typename Dt, typename Dt2 = Dt>
	Self &AddMember(const char *name, member_function<T, Dt> getter, member_function<T, void, Dt2> setter = nullptr)
	{
		lua_State *L = m_lua;
		GetAttrTable(L, m_index);

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushPointerToMember(L, getter);
		PushPointerToMember(L, setter);
		lua_pushcclosure(L, &getter_member_fn_wrapper_<T, Dt, Dt2>, 3);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, -2, name);
		lua_pop(L, 1);

		return *this;
	}

	// Magic to allow binding a const function to Lua. Take care to ensure that you do not
	// push a const object to lua, or this code will become undefined behavior.
	template <typename Rt, typename... Args>
	Self &AddFunction(const char *name, const_member_function<T, Rt, Args...> fn)
	{
		return AddFunction(name, reinterpret_cast<member_function<T, Rt, Args...>>(fn));
	}

	// Bind a member function to Lua.
	// Parameters will automatically be pulled from Lua and be passed to the function.
	// It is the responsiblity of the programmer to ensure a valid LuaPull implementation
	// is available for each parameter's time.
	// If the function has a non-void return type, its return value will automatically be
	// pushed to Lua.
	template <typename Rt, typename... Args>
	Self &AddFunction(const char *name, member_function<T, Rt, Args...> fn)
	{
		lua_State *L = m_lua;
		GetMethodTable(L, m_index);

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushPointerToMember(L, fn);
		lua_pushcclosure(L, &member_fn_wrapper_<T, Rt, Args...>, 2);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, -2, name);
		lua_pop(L, 1);

		return *this;
	}

	// Bind a free function to Lua.
	// The self parameter will be automatically provided, but the function
	// is responsible for pulling the rest of its parameters and pushing the
	// appropriate number of return values.
	Self &AddFunction(const char *name, member_cfunction<T> fn)
	{
		lua_State *L = m_lua;
		GetMethodTable(L, m_index);

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushFreeFunction(L, fn);
		lua_pushcclosure(L, &member_cfn_wrapper_<T>, 2);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, -2, name);
		lua_pop(L, 1);

		return *this;
	}

	// Add a raw lua function to this object's method table
	// For e.g. static member functions
	Self &AddFunction(const char *name, lua_CFunction func)
	{
		GetMethodTable(m_lua, m_index);

		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, -2, name);
		lua_pop(m_lua, 1);
		return *this;
	}

	// Magic to allow binding a const function to Lua. Take care to ensure that you do not
	// push a const object to lua, or this code will become undefined behavior.
	template <typename Rt, typename... Args>
	Self &AddMeta(const char *name, const_member_function<T, Rt, Args...> fn)
	{
		return AddMeta(name, reinterpret_cast<member_function<T, Rt, Args...>>(fn));
	}

	// Bind a member function to Lua as a metamethod.
	// Parameters will automatically be pulled from Lua and be passed to the function.
	// It is the responsiblity of the programmer to ensure a valid LuaPull implementation
	// is available for each parameter's time.
	// If the function has a non-void return type, its return value will automatically be
	// pushed to Lua.
	template <typename Rt, typename... Args>
	Self &AddMeta(const char *name, member_function<T, Rt, Args...> fn)
	{
		lua_State *L = m_lua;

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushPointerToMember(L, fn);
		lua_pushcclosure(L, &member_fn_wrapper_<T, Rt, Args...>, 2);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, m_index, name);

		return *this;
	}

	// Bind a free function to Lua as a metamethod.
	// The self parameter will be automatically provided, but the function
	// is responsible for pulling the rest of its parameters and pushing the
	// appropriate number of return values.
	Self &AddMeta(const char *name, member_cfunction<T> fn)
	{
		lua_State *L = m_lua;

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushFreeFunction(L, fn);
		lua_pushcclosure(L, &member_cfn_wrapper_<T>, 2);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, m_index, name);

		return *this;
	}

	// Add a raw lua function to this object's metamethod table
	Self &AddMeta(const char *name, lua_CFunction func)
	{
		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, m_index, name);

		return *this;
	}

	Self &RegisterFuncs(const luaL_Reg *functions)
	{
		lua_State *L = m_lua;
		GetMethodTable(L, m_index);

		for (const luaL_Reg *func = functions; func->name; ++func) {
			lua_pushcfunction(L, func->func);
			if (m_protected)
				lua_pushcclosure(L, secure_trampoline, 1);

			lua_setfield(L, -2, func->name);
		}

		lua_pop(L, 1);
	}

	Self &AddCallCtor(lua_CFunction func)
	{
		GetMethodTable(m_lua, m_index);
		lua_getmetatable(m_lua, -1);

		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, -2, "__call");
		lua_pop(m_lua, 2);

		return *this;
	}

	Self &AddNewCtor(lua_CFunction func)
	{
		GetMethodTable(m_lua, m_index);

		lua_pushcfunction(m_lua, func);
		if (m_protected)
			lua_pushcclosure(m_lua, secure_trampoline, 1);

		lua_setfield(m_lua, -2, "New");
		lua_pop(m_lua, 1);
		return *this;
	}

private:
	bool m_protected = false;
};
