// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUASERIALIZER_H
#define _LUASERIALIZER_H

#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaRef.h"
#include "DeleteEmitter.h"

class LuaSerializer : public DeleteEmitter {
	friend class LuaObject<LuaSerializer>;
	friend void LuaRef::SaveToJson(Json::Value &jsonObj);
	friend void LuaRef::LoadFromJson(const Json::Value &jsonObj);

public:
	void ToJson(Json::Value &jsonObj);
	void FromJson(const Json::Value &jsonObj);

	void InitTableRefs();
	void UninitTableRefs();

private:
	static int l_register(lua_State *l);
	static int l_register_class(lua_State *l);

	static void pickle(lua_State *l, int idx, std::string &out, std::string key = "");
	static const char *unpickle(lua_State *l, const char *pos);
};

#endif
