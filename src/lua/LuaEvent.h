// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAEVENT_H
#define _LUAEVENT_H

#include "DeleteEmitter.h"
#include "Lua.h"
#include "LuaObject.h"
#include "LuaPushPull.h"
#include "Pi.h"

namespace LuaEvent {

	class ArgsBase {
	public:
		virtual ~ArgsBase() {}

		virtual void PrepareStack(lua_State *l) const = 0;
	};

	class EmptyArgs : public ArgsBase {
	public:
		inline void PrepareStack(lua_State *l) const {}
	};

	template <typename...> class Args;

	template <typename T0, typename T1, typename T2>
	class Args<T0, T1, T2> : public ArgsBase {
	public:
		Args(T0 _arg0, T1 _arg1, T2 _arg2) :
			arg0(_arg0),
			arg1(_arg1),
			arg2(_arg2) {}
		virtual ~Args() {}

		T0 arg0;
		T1 arg1;
		T2 arg2;

		inline void PrepareStack(lua_State *l) const
		{
			LuaPush<T0>(l, arg0);
			LuaPush<T1>(l, arg1);
			LuaPush<T2>(l, arg2);
		}
	};

	template <typename T0, typename T1>
	class Args<T0, T1> : public ArgsBase {
	public:
		Args(T0 _arg0, T1 _arg1) :
			arg0(_arg0),
			arg1(_arg1) {}
		virtual ~Args() {}

		T0 arg0;
		T1 arg1;

		inline void PrepareStack(lua_State *l) const
		{
			LuaPush<T0>(l, arg0);
			LuaPush<T1>(l, arg1);
		}
	};

	template <typename T0>
	class Args<T0> : public ArgsBase {
	public:
		Args(T0 _arg0) :
			arg0(_arg0) {}
		virtual ~Args() {}

		T0 arg0;

		inline void PrepareStack(lua_State *l) const
		{
			LuaPush<T0>(l, arg0);
		}
	};

	void Clear();
	void Emit();

	void QueueInternal(const char *event, const ArgsBase &args);

	template <typename... TArgs>
	inline void Queue(const char *event, TArgs... args)
	{
		QueueInternal(event, Args<TArgs...>(args...));
	}

	inline void Queue(const char *event)
	{
		QueueInternal(event, EmptyArgs());
	}
} // namespace LuaEvent

#endif
