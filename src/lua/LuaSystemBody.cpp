// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Body.h"
#include "EnumStrings.h"
#include "Game.h"
#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "SectorView.h"
#include "Space.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystem.h"

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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushinteger(l, sbody->GetPath().bodyIndex);
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushstring(l, sbody->GetName().c_str());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushstring(l, EnumStrings::GetString("BodyType", sbody->GetType()));
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushstring(l, EnumStrings::GetString("BodySuperType", sbody->GetSuperType()));
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushinteger(l, sbody->GetSeed());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);

	if (!sbody->GetStarSystem()) {
		// orphan, its system has already been deleted, but SystemPath remains
		// we'll make a new one just like it
		RefCountedPtr<StarSystem> s = Pi::game->GetGalaxy()->GetStarSystem(sbody->GetPath());
		sbody = s->GetBodyByPath(sbody->GetPath());
		LuaObject<SystemBody>::PushToLua(sbody->GetParent());
	} else {
		LuaObject<SystemBody>::PushToLua(sbody->GetParent());
	}
	return 1;
}

/*
 * Attribute: system
 *
 * The StarSystem which contains this SystemBody
 *
 * Availability:
 *
 *   alpha 16
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_system(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	LuaPush(l, sbody->GetStarSystem());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetPopulation());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetMass());
	return 1;
}

/*
 * Attribute: gravity
 *
 * The gravity on the surface of the body (m/s).
 *
 * Availability:
 *
 *   alpha 21
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_gravity(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->CalcSurfaceGravity());
	return 1;
}

/*
 * Attribute: escapeVelocity
 *
 * The speed an object need to break free from the gravitational influence
 * of a body and leave it behind with no further acceleration.
 *
 * Availability:
 *
 *   July 2023
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_escape_velocity(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->CalcEscapeVelocity());
	return 1;
}

/*
 * Attribute: meanDensity
 *
 * The mean density of a body (kg/m3).
 *
 * Availability:
 *
 *   July 2023
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_mean_density(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->CalcMeanDensity());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetOrbMin() * AU);
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetOrbMax() * AU);
	return 1;
}

/*
 * Attribute: orbitPeriod
 *
 * The orbit of the body, around its parent, in days, as a float
 *
 * Availability:
 *
 *   201708
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_orbital_period(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetOrbit().Period() / float(60 * 60 * 24));
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetRotationPeriodInDays());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetSemiMajorAxis() * AU);
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetEccentricity());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetAxialTilt());
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
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushinteger(l, sbody->GetAverageTemp());
	return 1;
}

/*
 * Attribute: metallicity
 *
 * Returns the measure of metallicity of the body
 * (crust) 0.0 = light (Al, SiO2, etc), 1.0 = heavy (Fe, heavy metals)
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_metallicity(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetMetallicity());
	return 1;
}

/*
 * Attribute: atmosDensity
 *
 * Returns the atmospheric density at "surface level" of the body
 * 0.0 = no atmosphere, 1.225 = earth atmosphere density, 64+ ~= venus
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_atmosDensity(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetAtmSurfaceDensity());
	return 1;
}

/*
 * Attribute: atmosOxidizing
 *
 * Returns the compositional value of any atmospheric gasses in the bodys atmosphere (if any)
 * 0.0 = reducing (H2, NH3, etc), 1.0 = oxidising (CO2, O2, etc)
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_atmosOxidizing(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetAtmosOxidizing());
	return 1;
}

/*
 * Attribute: surfacePressure
 *
 * The pressure of the atmosphere at the surface of the body (atm).
 *
 * Availability:
 *
 *   2024
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_surfacePressure(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetAtmSurfacePressure());
	return 1;
}

/*
 * Attribute: volatileLiquid
 *
 * Returns the measure of volatile liquids present on the body
 * 0.0 = none, 1.0 = waterworld (earth = 70%)
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_volatileLiquid(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetVolatileLiquid());
	return 1;
}

/*
 * Attribute: volatileIces
 *
 * Returns the measure of volatile ices present on the body
 * 0.0 = none, 1.0 = total ice cover (earth = 3%)
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_volatileIces(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetVolatileIces());
	return 1;
}

/*
 * Attribute: volcanicity
 *
 * Returns the measure of volcanicity of the body
 * 0.0 = none, 1.0 = lava planet
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_volcanicity(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetVolcanicity());
	return 1;
}

/*
 * Attribute: life
 *
 * Returns the measure of life present on the body
 * 0.0 = dead, 1.0 = teeming (~= pandora)
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_life(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetLife());
	return 1;
}

/*
 * Attribute: agricultural
 *
 * Returns the measure of agricultural activity present on the body
 * 0.0 = dead, 1.0 = teeming (~= breadbasket)
 *
 * Availability:
 *
 *   January 2023
 *
 * Status:
 *
 *   experimental
 */
static int l_sbody_attr_agricultural(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushnumber(l, sbody->GetAgriculturalAsFixed().ToDouble());
	return 1;
}

/*
 * Attribute: hasRings
 *
 * Returns true if the body has a ring or rings of debris or ice in orbit around it
 *
 * Availability:
 *
 *   January 2018
 *
 * Status:
 *
 *  experimental
 */

static int l_sbody_attr_has_rings(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushboolean(l, sbody->HasRings());
	return 1;
}

/*
 * Attribute: hasAtmosphere
 *
 * Returns true if an atmosphere is present, false if not
 *
 * Availability:
 *
 *   alpha 21
 *
 * Status:
 *
 *  experimental
 */

static int l_sbody_attr_has_atmosphere(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushboolean(l, sbody->HasAtmosphere());
	return 1;
}

/*
 * Attribute: isScoopable
 *
 * Returns true if the system body can be scoopable, false if not
 *
 * Availablility:
 *
 *   alpha 21
 *
 * Status:
 *
 *  experimental
 */

static int l_sbody_attr_is_scoopable(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	lua_pushboolean(l, sbody->IsScoopable());
	return 1;
}

static int l_sbody_attr_path(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	LuaObject<SystemPath>::PushToLua(sbody->GetPath());
	return 1;
}

static int l_sbody_attr_astro_description(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	LuaPush(l, sbody->GetAstroDescription());
	return 1;
}

static int l_sbody_attr_body(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	if (Pi::game) {
		Space *space = Pi::game->GetSpace();
		if (space) {
			const SystemPath &path = sbody->GetPath();
			Body *body = space->FindBodyForPath(&path);
			if (body) {
				LuaObject<Body>::PushToLua(body);
			} else {
				lua_pushnil(l);
			}
		}
	} else {
		lua_pushnil(l);
	}
	return 1;
}

static int l_sbody_attr_children(lua_State *l)
{
	SystemBody *sbody = LuaObject<SystemBody>::CheckFromLua(1);
	LuaTable children(l);
	int i = 1;
	for (auto child : sbody->GetChildren()) {
		LuaPush(l, i++);
		LuaObject<SystemBody>::PushToLua(child);
		lua_settable(l, -3);
	}
	return 1;
}

static int l_sbody_attr_nearest_jumpable(lua_State *l)
{
	double time = Pi::game->GetTime();
	LuaObject<SystemBody>::PushToLua(LuaObject<SystemBody>::CheckFromLua(1)->GetNearestJumpable(time));
	return 1;
}

static int l_sbody_attr_is_moon(lua_State *l)
{
	LuaPush<bool>(l, LuaObject<SystemBody>::CheckFromLua(1)->IsMoon());
	return 1;
}

static int l_sbody_attr_is_station(lua_State *l)
{
	LuaPush<bool>(l, LuaObject<SystemBody>::CheckFromLua(1)->GetSuperType() == SystemBody::SUPERTYPE_STARPORT);
	return 1;
}

static int l_sbody_attr_is_ground_station(lua_State *l)
{
	SystemBody *sb = LuaObject<SystemBody>::CheckFromLua(1);
	LuaPush<bool>(l, sb->GetSuperType() == SystemBody::SUPERTYPE_STARPORT && sb->GetType() == SystemBody::TYPE_STARPORT_SURFACE);
	return 1;
}

static int l_sbody_attr_is_space_station(lua_State *l)
{
	SystemBody *sb = LuaObject<SystemBody>::CheckFromLua(1);
	LuaPush<bool>(l, sb->GetSuperType() == SystemBody::SUPERTYPE_STARPORT && sb->GetType() == SystemBody::TYPE_STARPORT_ORBITAL);
	return 1;
}

static int l_sbody_attr_physics_body(lua_State *l)
{
	SystemBody *b = LuaObject<SystemBody>::CheckFromLua(1);

	Body *physbody = Pi::game->GetSpace()->FindBodyForPath(&b->GetPath());
	LuaObject<Body>::PushToLua(physbody);
	return 1;
}

template <>
const char *LuaObject<SystemBody>::s_type = "SystemBody";

template <>
void LuaObject<SystemBody>::RegisterClass()
{
	static const luaL_Reg l_attrs[] = {
		{ "index", l_sbody_attr_index },
		{ "name", l_sbody_attr_name },
		{ "type", l_sbody_attr_type },
		{ "superType", l_sbody_attr_super_type },
		{ "seed", l_sbody_attr_seed },
		{ "parent", l_sbody_attr_parent },
		{ "system", l_sbody_attr_system },
		{ "population", l_sbody_attr_population },
		{ "radius", l_sbody_attr_radius },
		{ "mass", l_sbody_attr_mass },
		{ "gravity", l_sbody_attr_gravity },
		{ "escapeVelocity", l_sbody_attr_escape_velocity },
		{ "meanDensity", l_sbody_attr_mean_density },
		{ "periapsis", l_sbody_attr_periapsis },
		{ "apoapsis", l_sbody_attr_apoapsis },
		{ "orbitPeriod", l_sbody_attr_orbital_period },
		{ "rotationPeriod", l_sbody_attr_rotation_period },
		{ "semiMajorAxis", l_sbody_attr_semi_major_axis },
		{ "eccentricity", l_sbody_attr_eccentricty },
		{ "axialTilt", l_sbody_attr_axial_tilt },
		{ "averageTemp", l_sbody_attr_average_temp },
		{ "metallicity", l_sbody_attr_metallicity },
		{ "atmosDensity", l_sbody_attr_atmosDensity },
		{ "atmosOxidizing", l_sbody_attr_atmosOxidizing },
		{ "surfacePressure", l_sbody_attr_surfacePressure },
		{ "volatileLiquid", l_sbody_attr_volatileLiquid },
		{ "volatileIces", l_sbody_attr_volatileIces },
		{ "volcanicity", l_sbody_attr_volcanicity },
		{ "life", l_sbody_attr_life },
		{ "hasRings", l_sbody_attr_has_rings },
		{ "agricultural", l_sbody_attr_agricultural },
		{ "hasAtmosphere", l_sbody_attr_has_atmosphere },
		{ "isScoopable", l_sbody_attr_is_scoopable },
		{ "astroDescription", l_sbody_attr_astro_description },
		{ "path", l_sbody_attr_path },
		{ "body", l_sbody_attr_body },
		{ "children", l_sbody_attr_children },
		{ "nearestJumpable", l_sbody_attr_nearest_jumpable },
		{ "isMoon", l_sbody_attr_is_moon },
		{ "isStation", l_sbody_attr_is_station },
		{ "isGroundStation", l_sbody_attr_is_ground_station },
		{ "isSpaceStation", l_sbody_attr_is_space_station },
		{ "physicsBody", l_sbody_attr_physics_body },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, 0, l_attrs, 0);
}
