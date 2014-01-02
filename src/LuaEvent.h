// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAEVENT_H
#define _LUAEVENT_H

#include "Lua.h"
#include "LuaObject.h"
#include "DeleteEmitter.h"
#include "Pi.h"

namespace LuaEvent {

	class ArgsBase {
	public:
		virtual ~ArgsBase() {}

		virtual void PrepareStack() const = 0;
	};

	template <typename T0=void, typename T1=void>
	class Args : public ArgsBase {
	public:
		Args(T0 *_arg0, T1 *_arg1) : arg0(_arg0), arg1(_arg1) { }
		virtual ~Args() {}

		T0 *arg0;
		T1 *arg1;

		inline void PrepareStack() const {
			LuaObject<T0>::PushToLua(arg0);
			LuaObject<T1>::PushToLua(arg1);
		}
	};

	template <typename T0>
	class Args<T0,void> : public ArgsBase {
	public:
		Args(T0 *_arg0) : arg0(_arg0) { }
		virtual ~Args() {}

		T0 *arg0;

		inline void PrepareStack() const {
			LuaObject<T0>::PushToLua(arg0);
		}
	};

	template <typename T0>
	class Args<T0,const char*> : public ArgsBase {
	public:
		Args(T0 *_arg0, const char *_arg1) : arg0(_arg0), arg1(_arg1) {}
		virtual ~Args() {}

		T0         *arg0;
		const char *arg1;

		inline void PrepareStack() const {
			LuaObject<T0>::PushToLua(arg0);
			lua_pushstring(Lua::manager->GetLuaState(), arg1);
		}
	};

	template <>
	class Args<void,void> : public ArgsBase {
	public:
		Args() {}
		virtual ~Args() {}

		inline void PrepareStack() const {}
	};

	void Clear();
	void Emit();

	void Queue(const char *event, const ArgsBase &args);

	template <typename T0, typename T1>
	void Queue(const char *event, T0 *arg0, T1 *arg1) {
		Queue(event, Args<T0, T1>(arg0, arg1));
	}

	template <typename T0>
	void Queue(const char *event, T0 *arg0, const char *arg1) {
		Queue(event, Args<T0, const char *>(arg0, arg1));
	}

	template <typename T0>
	void Queue(const char *event, T0 *arg0) {
		Queue(event, Args<T0>(arg0));
	}

	inline void Queue(const char *event) {
		Queue(event, Args<>());
	}
}

#endif
