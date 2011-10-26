#include "LuaObject.h"
#include "LuaSBody.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
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

/*
 * Attribute: type
 *
 * The type of the body, as a <Constants.BodyType> constant
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbody_attr_type(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "BodyType", sbody->type));
	return 1;
}

/*
 * Attribute: superType
 *
 * The supertype of the body, as a <Constants.BodySuperType> constant
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbody_attr_super_type(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "BodySuperType", sbody->GetSuperType()));
	return 1;
}

/*
 * Attribute: seed
 *
 * The random seed used to generate this <SystemBody>. This is guaranteed to
 * be the same for this body across runs of the same build of the game, and
 * should be used to seed a <Rand> object when you want to ensure the same
 * random numbers come out each time.
 *
 * This value is the same is the one available via <Body.seed> once you enter
 * this system.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbody_attr_seed(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushinteger(l, sbody->seed);
	return 1;
}

/*
 * Attribute: parent
 *
 * The parent of the body, as a <SystemBody>. A body orbits its parent.
 *
 * Availability:
 *
 *   alpha 14
 *
 * Status:
 *
 *   stable
 */
static int l_sbody_attr_parent(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	LuaSBody::PushToLua(sbody->parent);
	return 1;
}

/*
 * Attribute: population
 *
 * The population of the body, in billions of people.
 *
 * Availability:
 *
 *   not yet
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_population(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->m_population.ToDouble());
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBody> >::s_type = "SystemBody";

template <> void LuaObject<LuaUncopyable<SBody> >::RegisterClass()
{
	static const luaL_reg l_attrs[] = {
		{ "index",     l_sbody_attr_index      },
		{ "name",      l_sbody_attr_name       },
		{ "type",      l_sbody_attr_type       },
		{ "superType", l_sbody_attr_super_type },
		{ "seed",      l_sbody_attr_seed       },
		{ "parent",    l_sbody_attr_parent     },
		{ "population",l_sbody_attr_population },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, NULL, l_attrs, NULL);
}
