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
	lua_pushinteger(l, sbody->path.bodyIndex);
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

	// sbody->parent is 0 as it was cleared by the acquirer. we need to go
	// back to the starsystem proper to get what we need.
	RefCountedPtr<StarSystem> s = StarSystem::GetCached(sbody->path);
	SBody *live_sbody = s->GetBodyByPath(sbody->path);
	LuaSBody::PushToLua(live_sbody->parent);
	return 1;
}

/*
 * Attribute: population
 *
 * The population of the body, in billions of people.
 *
 * Availability:
 *
 *   alpha 16
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

/*
 * Attribute: radius
 *
 * The radius of the body, in metres (m).
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_radius(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->GetRadius());
	return 1;
}

/*
 * Attribute: mass
 *
 * The mass of the body, in kilograms (kg).
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_mass(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->GetMass());
	return 1;
}

/*
 * Attribute: periapsis
 *
 * The periapsis of the body's orbit, in metres (m).
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_periapsis(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->orbMin.ToDouble()*AU);
	return 1;
}

/*
 * Attribute: apoapsis
 *
 * The apoapsis of the body's orbit, in metres (m).
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_apoapsis(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->orbMax.ToDouble()*AU);
	return 1;
}

/*
 * Attribute: rotationPeriod
 *
 * The rotation period of the body, in days
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_rotation_period(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->rotationPeriod.ToDouble());
	return 1;
}

/*
 * Attribute: semiMajorAxis
 *
 * The semi-major axis of the orbit, in metres (m).
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_semi_major_axis(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->semiMajorAxis.ToDouble()*AU);
	return 1;
}

/*
 * Attribute: eccentricity
 *
 * The orbital eccentricity of the body
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_eccentricty(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->eccentricity.ToDouble());
	return 1;
}

/*
 * Attribute: axialTilt
 *
 * The axial tilt of the body, in radians
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_axial_tilt(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushnumber(l, sbody->axialTilt.ToDouble());
	return 1;
}

/*
 * Attribute: averageTemp
 *
 * The average surface temperature of the body, in degrees kelvin
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_average_temp(lua_State *l)
{
	SBody *sbody = LuaSBody::GetFromLua(1);
	lua_pushinteger(l, sbody->averageTemp);
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBody> >::s_type = "SystemBody";

template <> void LuaObject<LuaUncopyable<SBody> >::RegisterClass()
{
	static const luaL_reg l_attrs[] = {
		{ "index",          l_sbody_attr_index           },
		{ "name",           l_sbody_attr_name            },
		{ "type",           l_sbody_attr_type            },
		{ "superType",      l_sbody_attr_super_type      },
		{ "seed",           l_sbody_attr_seed            },
		{ "parent",         l_sbody_attr_parent          },
		{ "population",     l_sbody_attr_population      },
		{ "radius",         l_sbody_attr_radius          },
		{ "mass",           l_sbody_attr_mass            },
		{ "periapsis",      l_sbody_attr_periapsis       },
		{ "apoapsis",       l_sbody_attr_apoapsis        },
		{ "rotationPeriod", l_sbody_attr_rotation_period },
		{ "semiMajorAxis",  l_sbody_attr_semi_major_axis },
		{ "eccentricity",   l_sbody_attr_eccentricty     },
		{ "axialTilt",      l_sbody_attr_axial_tilt      },
		{ "averageTemp",    l_sbody_attr_average_temp    },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, NULL, l_attrs, NULL);
}
