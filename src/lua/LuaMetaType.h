// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Lua.h"
#include "LuaCall.h"
#include "LuaManager.h"
#include "LuaPushPull.h"
#include "LuaTable.h"

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

protected:
	template <typename T, typename Dt>
	using member_pointer = Dt T::*;

	template <typename T>
	using free_function = int (*)(lua_State *, T *);

	template <typename T, typename Rt, typename... Args>
	using member_function = Rt (T::*)(Args...);

	template <typename T, typename Rt, typename... Args>
	using const_member_function = Rt (T::*)(Args...) const;

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

	template <typename T>
	static int getter_member_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));
		free_function<T> getter = PullFreeFunction<T>(L, lua_upvalueindex(2));

		if (!ptr)
			return luaL_error(L, "Null or invalid userdata accessed for property %s", name);

		if (lua_gettop(L) > 2)
			return luaL_error(L, "Invalid number of arguments for property getter/setter %s", name);

		if (lua_gettop(L) > 1) {
			free_function<T> setter = PullFreeFunction<T>(L, lua_upvalueindex(3));
			if (setter != nullptr)
				return setter(L, ptr);
			else
				return luaL_error(L, "Attempt to call undefined setter for property %s", name);
		}

		return getter(L, ptr);
	}

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

	template <typename T>
	static int fn_wrapper_(lua_State *L)
	{
		T *ptr = LuaPull<T *>(L, 1);
		const char *name = lua_tostring(L, lua_upvalueindex(1));
		luaL_checktype(L, lua_upvalueindex(2), LUA_TLIGHTUSERDATA);

		if (!ptr)
			return luaL_error(L, "Invalid userdata accessed for function %s", name);

		free_function<T> fn = PullFreeFunction<T>(L, lua_upvalueindex(2));
		return fn(L, ptr);
	}

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
	static void PushFreeFunction(lua_State *L, free_function<T> obj)
	{
		static_assert(sizeof(free_function<T>) == sizeof(void *), "Free functions cannot be 'fat' lambdas!");
		lua_pushlightuserdata(L, reinterpret_cast<void *>(obj));
	}

	template <typename T>
	static free_function<T> PullFreeFunction(lua_State *L, int index)
	{
		return reinterpret_cast<free_function<T>>(lua_touserdata(L, index));
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

private:
	bool m_protected = false;
};

template <typename T>
class LuaMetaType : public LuaMetaTypeBase {
public:
	LuaMetaType(const char *name) :
		LuaMetaTypeBase(name)
	{}

	LuaMetaType &StartRecording()
	{
		LuaMetaTypeBase::StartRecording();
		return *this;
	}

	LuaMetaType &StopRecording()
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
	LuaMetaType &AddMember(const char *name, member_pointer<T, Dt> t)
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
	LuaMetaType &AddMember(const char *name, free_function<T> getter, free_function<T> setter = nullptr)
	{
		lua_State *L = m_lua;
		GetAttrTable(L, m_index);

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushFreeFunction(L, getter);
		PushFreeFunction(L, setter);
		lua_pushcclosure(L, &getter_member_wrapper_<T>, 3);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, -2, name);
		lua_pop(L, 1);

		return *this;
	}

	// Magic to allow binding a const function to Lua. Take care to ensure that you do not
	// push a const object to lua, or this code will become undefined behavior.
	template <typename Dt, typename Dt2 = Dt>
	LuaMetaType &AddMember(const char *name, const_member_function<T, Dt> getter, member_function<T, void, Dt2> setter = nullptr)
	{
		return AddMember(name, reinterpret_cast<member_function<T, Dt>>(getter), setter);
	}

	// Bind a pseudo-member to Lua via a member-function getter and setter.
	// The parameter will automatically be pulled from Lua and passed to the setter.
	template <typename Dt, typename Dt2 = Dt>
	LuaMetaType &AddMember(const char *name, member_function<T, Dt> getter, member_function<T, void, Dt2> setter = nullptr)
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
	LuaMetaType &AddFunction(const char *name, const_member_function<T, Rt, Args...> fn)
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
	LuaMetaType &AddFunction(const char *name, member_function<T, Rt, Args...> fn)
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
	LuaMetaType &AddFunction(const char *name, free_function<T> fn)
	{
		lua_State *L = m_lua;
		GetMethodTable(L, m_index);

		lua_pushstring(L, (m_typeName + "." + name).c_str());
		PushFreeFunction(L, fn);
		lua_pushcclosure(L, &fn_wrapper_<T>, 2);
		if (m_protected)
			lua_pushcclosure(L, &secure_trampoline, 1);

		lua_setfield(L, -2, name);
		lua_pop(L, 1);

		return *this;
	}

	LuaMetaType &RegisterFuncs(const luaL_Reg *functions)
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

private:
	bool m_protected = false;
};
