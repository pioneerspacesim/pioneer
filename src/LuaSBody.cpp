#include "LuaObject.h"
#include "LuaSBody.h"
#include "LuaUtils.h"
#include "StarSystem.h"

/*
 * Class: SystemBody
 *
 * Class representing a system body.
 *
 * <SystemBody> differs from <Body> in that it holds the properties that are
 * used to generate the physics <body> that is created when the player enters
 * a system. It exists outside of the current space. That is, scripts can use
 * a <SystemBody> to discover information about a body that exists in another
 * system.
 */

/*
 * Attribute: index
 *
 * The body index of the body in its system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbody_attr_index(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushinteger(l, sbody->id);
	return 1;
}

/*
 * Attribute: name
 *
 * The name of the body
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbody_attr_name(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushstring(l, sbody->name.c_str());
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBody> >::s_type = "SystemBody";

template <> void LuaObject<LuaUncopyable<SBody> >::RegisterClass()
{
	static const luaL_reg l_attrs[] = {
        { "index", l_sbody_attr_index },
        { "name",  l_sbody_attr_name  },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, NULL, l_attrs, NULL);
}
