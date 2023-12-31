// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "BodyComponent.h"
#include "LuaObject.h"

// Type-specific Lua management implementation for body components.
//
// BodyComponents are not automatically compatible with LuaPush / LuaPull,
// and must either be managed with PushComponentToLua() or retrieved with
// GetComponent(typeName) from the Lua side.
//
// Note: there is currently no Lua event handling for when a C++ component is
// deleted - the LuaObject handle is just silently orphaned.
template <typename T>
struct BodyComponentDB::LuaInterface final : LuaInterfaceBase {
	static_assert(std::is_base_of_v<LuaWrappable, T>, "BodyComponents must inherit from LuaWrappable to be pushed to Lua!");

	LuaInterface(Pool<T> *pool) :
		pool(pool)
	{}
	Pool<T> *pool;

	virtual void PushToLua(const Body *body) override
	{
		LuaObject<T>::PushComponentToLua(pool->get(body));
	}

	virtual void DeregisterComponent(const Body *body) override
	{
		LuaObjectBase::DeregisterObject(pool->get(body));
	}
};
