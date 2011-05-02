#include "LuaObject.h"
#include "LuaSBody.h"
#include "LuaUtils.h"
#include "StarSystem.h"

static int l_sbody_get_id(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushinteger(l, sbody->id);
	return 1;
}

static int l_sbody_get_name(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushstring(l, sbody->name.c_str());
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBody> >::s_type = "SystemBody";

template <> void LuaObject<LuaUncopyable<SBody> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
        { "GetId",   l_sbody_get_id   },
        { "GetName", l_sbody_get_name },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL, NULL);
}
