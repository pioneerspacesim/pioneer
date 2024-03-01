// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUASERIALIZER_H
#define _LUASERIALIZER_H

#include "DeleteEmitter.h"
#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaRef.h"

class Space;

class LuaSerializer : public DeleteEmitter {
	friend class LuaObjectBase;
	friend class LuaObject<LuaSerializer>;
	friend void LuaRef::SaveToJson(Json &jsonObj);
	friend void LuaRef::LoadFromJson(const Json &jsonObj);

public:
	void ToJson(Json &jsonObj);
	void FromJson(const Json &jsonObj);

	void SaveComponents(Json &jsonObj, Space *space);
	void LoadComponents(const Json &jsonObj, Space *space);

	void SavePersistent(Json &jsonObj);
	void LoadPersistent(const Json &jsonObj);

	void InitTableRefs();
	void UninitTableRefs();

private:
	static int l_register(lua_State *l);
	static int l_register_class(lua_State *l);
	static int l_register_persistent(lua_State *l);

	static void pickle_json(lua_State *l, int idx, Json &out, const std::string &key = "");
	static void unpickle_json(lua_State *l, const Json &value);
};

#endif
