// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_LUASIGNAL_H
#define UI_LUASIGNAL_H

namespace UI {

class LuaSignalTrampolineBase {
protected:
	static inline void _trampoline_start(lua_State *l, int idx) {
		lua_getfield(l, LUA_REGISTRYINDEX, "PiUISignal");
		assert(lua_istable(l, -1));

		lua_rawgeti(l, -1, idx);
		assert(lua_isfunction(l, -1));
	}

	static inline bool _trampoline_end(lua_State *l, int nargs) {
		pi_lua_protected_call(l, nargs, 1);
		bool ret = lua_toboolean(l, -1);
		lua_pop(l, 2);
		return ret;
	}
};

template <typename T0, typename T1>
class LuaSignalTrampoline : public LuaSignalTrampolineBase {
public:
	static inline bool trampoline(T0 arg0, T1 arg1, lua_State *l, int idx) {
		_trampoline_start(l, idx);
		arg0.ToLuaTable(l);
		arg1.ToLuaTable(l);
		return _trampoline_end(l, 2);
	}

	static inline void trampoline_void(T0 arg0, T1 arg1, lua_State *l, int idx) {
		trampoline(arg0, arg1, l, idx);
	}
};

template <typename T0>
class LuaSignalTrampoline<T0,sigc::nil> : public LuaSignalTrampolineBase {
public:
	static inline bool trampoline(T0 arg0, lua_State *l, int idx) {
		_trampoline_start(l, idx);
		arg0.ToLuaTable(l);
		return _trampoline_end(l, 1);
	}

    static inline void trampoline_void(T0 arg0, lua_State *l, int idx) {
        trampoline(arg0, l, idx);
    }
};

template <>
class LuaSignalTrampoline<sigc::nil,sigc::nil> : public LuaSignalTrampolineBase {
public:
	static inline bool trampoline(lua_State *l, int idx) {
		_trampoline_start(l, idx);
		return _trampoline_end(l, 0);
	}

	static inline void trampoline_void(lua_State *l, int idx) {
		trampoline(l, idx);
	}
};

template <>
class LuaSignalTrampoline<unsigned int,const std::string &> : public LuaSignalTrampolineBase {
public:
	static inline bool trampoline(unsigned int arg0, const std::string &arg1, lua_State *l, int idx) {
		_trampoline_start(l, idx);
		lua_pushinteger(l, arg0);
		lua_pushlstring(l, arg1.c_str(), arg1.size());
		return _trampoline_end(l, 2);
	}

	static inline void trampoline_void(unsigned int arg0, const std::string &arg1, lua_State *l, int idx) {
		trampoline(arg0, arg1, l, idx);
	}
};

template <>
class LuaSignalTrampoline<const std::string &,sigc::nil> : public LuaSignalTrampolineBase {
public:
	static inline bool trampoline(const std::string &arg0, lua_State *l, int idx) {
		_trampoline_start(l, idx);
		lua_pushlstring(l, arg0.c_str(), arg0.size());
		return _trampoline_end(l, 1);
	}

	static inline void trampoline_void(const std::string &arg0, lua_State *l, int idx) {
		trampoline(arg0, l, idx);
	}
};

class LuaSignalBase {
protected:
	static inline void *connect_base(lua_State *l, int &idx) {
		luaL_checktype(l, 1, LUA_TTABLE);
		luaL_checktype(l, 2, LUA_TFUNCTION);

		lua_pushstring(l, "__signal");
		lua_rawget(l, 1);
		if (!lua_islightuserdata(l, -1)) {
			luaL_error(l, "signal pointer not found");
			return 0;
		}
		void *signal = lua_touserdata(l, -1);
		lua_pop(l, 1);

		lua_getfield(l, LUA_REGISTRYINDEX, "PiUISignal");
		if (lua_isnil(l, -1)) {
			lua_newtable(l);
			lua_pushvalue(l, -1);
			lua_setfield(l, LUA_REGISTRYINDEX, "PiUISignal");
		}

		idx = lua_rawlen(l, -1)+1;
		lua_pushvalue(l, 2);
		lua_rawseti(l, -2, idx);

		return signal;
	}
};

template <typename T0 = sigc::nil, typename T1 = sigc::nil>
class LuaSignal : public LuaSignalBase {
public:

	typedef typename sigc::signal<void,T0,T1> signal_type;

	static inline int l_connect(lua_State *l) {
		int idx;
		if (signal_type *signal = static_cast<signal_type *>(connect_base(l, idx)))
			signal->connect(sigc::bind(sigc::ptr_fun(&LuaSignalTrampoline<T0,T1>::trampoline_void), l, idx));
		return 0;
	}

	inline void Wrap(lua_State *l, signal_type &signal) {
		lua_newtable(l);

		lua_pushstring(l, "Connect");
		lua_pushcfunction(l, l_connect);
		lua_rawset(l, -3);

		lua_pushstring(l, "__signal");
		lua_pushlightuserdata(l, &signal);
		lua_rawset(l, -3);
	}
};

template <typename T0 = sigc::nil, typename T1 = sigc::nil>
class LuaSignalAccumulated : public LuaSignalBase {
public:

	typedef typename sigc::signal<bool,T0,T1>::template accumulated<UI::Widget::EventHandlerResultAccumulator> signal_type;

	static inline int l_connect(lua_State *l) {
		int idx;
		if (signal_type *signal = static_cast<signal_type *>(connect_base(l, idx)))
			signal->connect(sigc::bind(sigc::ptr_fun(&LuaSignalTrampoline<T0,T1>::trampoline), l, idx));
		return 0;
	}

	inline void Wrap(lua_State *l, signal_type &signal) {
		lua_newtable(l);

		lua_pushstring(l, "Connect");
		lua_pushcfunction(l, l_connect);
		lua_rawset(l, -3);

		lua_pushstring(l, "__signal");
		lua_pushlightuserdata(l, &signal);
		lua_rawset(l, -3);
	}
};

}

#endif
