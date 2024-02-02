// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Body.h"
#include "EnumStrings.h"
#include "Frame.h"
#include "Game.h"
#include "Json.h"
#include "LuaConstants.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaVector.h"
#include "Pi.h"
#include "Space.h"
#include "TerrainBody.h"
#include "WorldView.h"
#include "galaxy/SystemBody.h"

#include "CargoBody.h"
#include "HyperspaceCloud.h"
#include "Missile.h"
#include "ModelBody.h"
#include "Planet.h"
#include "Player.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "Star.h"

// For LuaFlags<ObjectType>
#include "enum_table.h"
#include "pigui/LuaFlags.h"

namespace PiGui {
	// Declared in LuaPiGuiInternal.h
	extern bool first_body_is_more_important_than(Body *, Body *);
	extern int pushOnScreenPositionDirection(lua_State *l, vector3d position);
} // namespace PiGui

static LuaFlags<ObjectType> s_bodyFlags ({
	{ "Body", ObjectType::BODY },
	{ "ModelBody", ObjectType::MODELBODY },
	{ "Ship", ObjectType::SHIP },
	{ "Player", ObjectType::PLAYER },
	{ "SpaceStation", ObjectType::SPACESTATION },
	{ "TerrainBody", ObjectType::TERRAINBODY },
	{ "Planet", ObjectType::PLANET },
	{ "Star", ObjectType::STAR },
	{ "CargoBody", ObjectType::CARGOBODY },
	{ "Projectile", ObjectType::PROJECTILE },
	{ "Missile", ObjectType::MISSILE },
	{ "HyperspaceCloud", ObjectType::HYPERSPACECLOUD }
});

/*
 * Class: Body
 *
 * Class represents a physical body.
 *
 * These objects only exist for the bodies of the system that the player is
 * currently in. If you need to retain a reference to a body outside of the
 * current system, look at <SystemBody>, <SystemPath> and the discussion of
 * <IsDynamic>.
 */

/*
 * Attribute: label
 *
 * The label for the body. This is what is displayed in the HUD and usually
 * matches the name of the planet, space station, etc if appropriate.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */

/*
 * Attribute: seed
 *
 * The random seed used to generate this <Body>. This is guaranteed to be the
 * same for this body across runs of the same build of the game, and should be
 * used to seed a <Rand> object when you want to ensure the same random
 * numbers come out each time.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_body_attr_seed(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);

	const SystemBody *sbody = b->GetSystemBody();
	assert(sbody);

	lua_pushnumber(l, sbody->GetSeed());
	return 1;
}

/*
 * Attribute: path
 *
 * The <SystemPath> that points to this body.
 *
 * If the body is a dynamic body it has no persistent path data, and its
 * <path> value will be nil.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_body_attr_path(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);

	const SystemBody *sbody = b->GetSystemBody();
	if (!sbody) {
		lua_pushnil(l);
		return 1;
	}

	const SystemPath path(sbody->GetPath());
	LuaObject<SystemPath>::PushToLua(path);

	return 1;
}

/*
 * Function: GetComponent
 *
 * Returns a handle to the specified LuaComponent or C++ Component attached
 * to the given body, if present. Will return nil if the specified component
 * does not exist on this body.
 *
 * Availability:
 *
 *   June 2022
 *
 * Status:
 *
 *   stable
 */
static int l_body_get_component(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	std::string componentName = luaL_checkstring(l, 2);

	BodyComponentDB::PoolBase *pool = BodyComponentDB::GetComponentType(componentName);
	if (pool && pool->luaInterface) {
		pool->luaInterface->PushToLua(b);
		return 1;
	}

	if (componentName == "__properties") {
		lua_pushnil(l);
		return 1;
	}

	lua_getuservalue(l, 1);
	lua_pushvalue(l, 2);
	lua_rawget(l, -2);

	return 1;
}

/*
 * Function: SetComponent
 *
 * Sets the given LuaComponent slot to a specific table value. Will error if
 * the slot name is reserved by C++ components or if the caller is attempting
 * to set the `__properties` reserved name.
 *
 * Parameters:
 *
 *   comp - a table object to be used as the component value or nil to remove
 *          the component from the Body.
 *
 * Availability:
 *
 *   June 2022
 *
 * Status:
 *
 *   stable
 */
static int l_body_set_component(lua_State *l)
{
	LuaObject<Body>::CheckFromLua(1);
	std::string componentName = luaL_checkstring(l, 2);

	BodyComponentDB::PoolBase *pool = BodyComponentDB::GetComponentType(componentName);
	if (pool) {
		return luaL_error(l, "Cannot set C++ body component '%s' to a lua value.");
	}

	if (componentName == "__properties") {
		return luaL_error(l, "'__properties' is a reserved name and cannot be used as a component.");
	}

	int type = lua_type(l, 3);
	if (type != LUA_TNIL && type != LUA_TTABLE) {
		return luaL_error(l, "Cannot set body component '%s' to non-table value.", componentName.data());
	}

	lua_getuservalue(l, 1);
	lua_replace(l, 1);
	lua_rawset(l, -3);

	return 0;
}

/*
 * Method: GetVelocityRelTo
 *
 * Get the body's velocity relative to another body as a Vector
 *
 * > body:GetVelocityRelTo(otherBody)
 *
 * Parameters:
 *
 *   other - the other body
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_body_get_velocity_rel_to(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	const Body *other = LuaObject<Body>::CheckFromLua(2);
	vector3d velocity = b->GetVelocityRelTo(other);
	LuaPush(l, velocity);
	return 1;
}

static int l_body_is_moon(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	const SystemBody *sb = body->GetSystemBody();
	if (!sb) {
		LuaPush<bool>(l, false);
	} else {
		LuaPush<bool>(l, sb->IsMoon());
	}
	return 1;
}

static int l_body_is_missile(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	LuaPush<bool>(l, body->GetType() == ObjectType::MISSILE);
	return 1;
}

static int l_body_is_station(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	LuaPush<bool>(l, body->GetType() == ObjectType::SPACESTATION);
	return 1;
}

static int l_body_is_space_station(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	const SystemBody *sb = body->GetSystemBody();
	LuaPush<bool>(l, sb ? sb->GetType() == SystemBody::BodyType::TYPE_STARPORT_ORBITAL : false);
	return 1;
}

static int l_body_is_ground_station(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	const SystemBody *sb = body->GetSystemBody();
	LuaPush<bool>(l, sb ? sb->GetType() == SystemBody::BodyType::TYPE_STARPORT_SURFACE : false);
	return 1;
}

static int l_body_is_cargo_container(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	LuaPush<bool>(l, body->GetType() == ObjectType::CARGOBODY);
	return 1;
}

static int l_body_is_ship(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	LuaPush<bool>(l, body->GetType() == ObjectType::SHIP);
	return 1;
}

static int l_body_is_hyperspace_cloud(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	LuaPush<bool>(l, body->GetType() == ObjectType::HYPERSPACECLOUD);
	return 1;
}

static int l_body_is_planet(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	const SystemBody *sb = body->GetSystemBody();
	if (!sb) {
		LuaPush<bool>(l, false);
	} else {
		LuaPush<bool>(l, sb->IsPlanet());
	}
	return 1;
}

static int l_body_get_system_body(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	SystemBody *sb = const_cast<SystemBody *>(body->GetSystemBody()); // TODO: ugly, change this...
	if (!sb) {
		lua_pushnil(l);
	} else {
		LuaObject<SystemBody>::PushToLua(sb);
	}
	return 1;
}

static int l_body_is_more_important_than(lua_State *l)
{
	Body *body = LuaObject<Body>::CheckFromLua(1);
	Body *other = LuaObject<Body>::CheckFromLua(2);

	// compare body and other
	// push true if body is "more important" than other
	// the most important body is shown on the hud and
	// bodies are sorted by importance in menus

	if (body == other) {
		LuaPush<bool>(l, false);
		return 1;
	}
	LuaPush<bool>(l, PiGui::first_body_is_more_important_than(body, other));
	return 1;
}

/*
 * Method: GetPositionRelTo
 *
 * Get the body's position relative to another body as a Vector
 *
 * > body:GetPositionRelTo(otherBody)
 *
 * Parameters:
 *
 *   other - the other body
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_body_get_position_rel_to(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	const Body *other = LuaObject<Body>::CheckFromLua(2);
	vector3d velocity = b->GetPositionRelTo(other);
	LuaPush<vector3d>(l, velocity);
	return 1;
}

/*
 * Method: GetAltitudeRelTo
 *
 * Get the body's altitude relative to another body
 *
 * Parameters:
 *
 *   other - the other body
 *   (optional) terrainRelative - what altitude to calculate (sea-level(false), above-terrain(true))
 *   if not specified, sea-level for distant bodies, above-ground otherwise
 *
 * Examples:
 *
 * > -- Get relative altitude to other body
 * > body:GetAltitudeRelTo(otherBody)
 *
 * > -- Get relative above terrain altitude to other body
 * > body:GetAltitudeRelTo(otherBody, true)
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_body_get_altitude_rel_to(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	const Body *other = LuaObject<Body>::CheckFromLua(2);
	AltitudeType altType = AltitudeType::DEFAULT;
	if (!lua_isnoneornil(l, 3)) {
		bool terrainRelative = lua_toboolean(l, 3);
		altType = terrainRelative ? AltitudeType::ABOVE_TERRAIN : AltitudeType::SEA_LEVEL;
	}
	double altitude = b->GetAltitudeRelTo(other, altType);
	LuaPush(l, altitude);
	return 1;
}

/*
 * Attribute: type
 *
 * The type of the body, as a <Constants.BodyType> constant.
 *
 * Only valid for non-dynamic <Bodies>. For dynamic bodies <type> will be nil.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_body_attr_type(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	const SystemBody *sbody = b->GetSystemBody();
	if (!sbody) {
		lua_pushnil(l);
		return 1;
	}

	lua_pushstring(l, EnumStrings::GetString("BodyType", sbody->GetType()));
	return 1;
}

/*
 * Attribute: superType
 *
 * The supertype of the body, as a <Constants.BodySuperType> constant
 *
 * Only valid for non-dynamic <Bodies>. For dynamic bodies <superType> will be nil.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_body_attr_super_type(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	const SystemBody *sbody = b->GetSystemBody();
	if (!sbody) {
		lua_pushnil(l);
		return 1;
	}

	lua_pushstring(l, EnumStrings::GetString("BodySuperType", sbody->GetSuperType()));
	return 1;
}

/*
 * Attribute: frameBody
 *
 * The non-dynamic body attached to the frame this dynamic body is in.
 *
 * Only valid for dynamic <Bodies>. For non-dynamic bodies <frameBody> will be
 * nil.
 *
 * <frameBody> can also be nil if this dynamic body is in a frame with no
 * non-dynamic body. This most commonly occurs when the player is in
 * hyperspace.
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_body_attr_frame_body(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	if (!b->IsType(ObjectType::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = Frame::GetFrame(b->GetFrame());

	if (!f) {
		lua_pushnil(l);
		return 1;
	}

	LuaObject<Body>::PushToLua(f->GetBody());
	return 1;
}

/*
 * Attribute: frameRotating
 *
 * Whether the frame this dynamic body is in is a rotating frame.
 *
 * Only valid for dynamic <Bodies>. For non-dynamic bodies <frameRotating>
 * will be nil.
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_body_attr_frame_rotating(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	if (!b->IsType(ObjectType::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = Frame::GetFrame(b->GetFrame());
	if (!f) {
		lua_pushnil(l);
		return 1;
	}

	lua_pushboolean(l, f->IsRotFrame());
	return 1;
}

/*
 * Method: IsDynamic
 *
 * Determine if the body is a dynamic body
 *
 * > isdynamic = body:IsDynamic()
 *
 * A dynamic body is one that is not part of the generated system. Currently
 * <Ships> and <CargoBodies> are dynamic bodies. <Stars>, <Planets> and
 * <SpaceStations> are not.
 *
 * Being a dynamic body generally means that there is no way to reference the
 * body outside of the context of the current system. A planet, for example,
 * can always be referenced by its <SystemPath> (available via <Body.path>),
 * even from outside the system. A <Ship> however can not be referenced in
 * this way. If a script needs to retain information about a ship that is no
 * longer in the <Player's> current system it must manage this itself.
 *
 * The above list of static/dynamic bodies may change in the future. Scripts
 * should use this method to determine the difference rather than checking
 * types directly.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_body_is_dynamic(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	lua_pushboolean(l, b->IsType(ObjectType::DYNAMICBODY));
	return 1;
}

/*
 * Method: DistanceTo
 *
 * Calculate the distance between two bodies
 *
 * > dist = body:DistanceTo(otherbody)
 *
 * Parameters:
 *
 *   otherbody - the body to calculate the distance to
 *
 * Returns:
 *
 *   dist - distance between the two bodies in meters
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_body_distance_to(lua_State *l)
{
	Body *b1 = LuaObject<Body>::CheckFromLua(1);
	Body *b2 = LuaObject<Body>::CheckFromLua(2);
	if (!b1->IsInSpace())
		return luaL_error(l, "Body:DistanceTo() arg #1 is not in space (probably a ship in hyperspace)");
	if (!b2->IsInSpace())
		return luaL_error(l, "Body:DistanceTo() arg #2 is not in space (probably a ship in hyperspace)");
	lua_pushnumber(l, b1->GetPositionRelTo(b2).Length());
	return 1;
}

/*
 * Method: GetGroundPosition
 *
 * Get latitude, longitude and altitude of frame of ref of a dynamic body or nil if the body is not a dynamic body
 *
 * > latitude, longitude, altitude = body:GetGroundPosition()
 *
 * Parameters:
 *
 * (optional) terrainRelative - what altitude to calculate (sea-level(false), above-terrain(true))
 * if not specified, sea-level for distant bodies, above-terrain otherwise
 *
 * Returns:
 *
 *   latitude - the latitude of the body in radians
 *   longitude - the longitude of the body in radians
 *   altitude - altitude above the ground in meters
 *
 * Examples:
 *
 * > -- Get ground position of the player
 * > local lat, long, alt = Game.player:GetGroundPosition()
 * > lat = math.rad2deg(lat)
 * > long = math.rad2deg(long)
 *
 * > -- Get ground position of the player with sea-level altitude
 * > local lat, long, alt = Game.player:GetGroundPosition(false)
 * > lat = math.rad2deg(lat)
 * > long = math.rad2deg(long)
 *
 * Availability:
 *
 *   July 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_body_get_ground_position(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	AltitudeType altType = AltitudeType::DEFAULT;
	if (!lua_isnoneornil(l, 2)) {
		bool terrainRelative = lua_toboolean(l, 2);
		altType = terrainRelative ? AltitudeType::ABOVE_TERRAIN : AltitudeType::SEA_LEVEL;
	}

	if (!b->IsType(ObjectType::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = Frame::GetFrame(b->GetFrame());

	if (!f) {
		lua_pushnil(l);
		return 1;
	}
	Body *astro = f->GetBody();
	if (!astro) {
		lua_pushnil(l);
		return 1;
	}

	vector3d pos = b->GetPositionRelTo(astro);
	double latitude = atan2(pos.y, sqrt(pos.x * pos.x + pos.z * pos.z));
	double longitude = atan2(pos.x, pos.z);
	lua_pushnumber(l, latitude);
	lua_pushnumber(l, longitude);
	double altitude = b->GetAltitudeRelTo(astro, altType);
	lua_pushnumber(l, altitude);
	return 3;
}

/*
 * Method: FindNearestTo
 *
 * Find the nearest object of a <Constants.PhysicsObjectType> type
 *
 * > closestObject = body:FindNearestTo(physicsObjectType)
 *
 * Parameters:
 *
 *   physicsObjectType - The closest object of <Constants.PhysicsObjectType> type
 *
 * Returns:
 *
 *   closestObject - The object closest to the body of specified type
 *
 * Examples:
 *
 * > -- Get closest object to player of type:
 * > closestStar = Game.player:FindNearestTo("STAR")
 * > closestStation = Game.player:FindNearestTo("SPACESTATION")
 * > closestPlanet = Game.player:FindNearestTo("PLANET")
 *
 * Availability:
 *
 *   2014 April
 *
 * Status:
 *
 *   experimental
 */
static int l_body_find_nearest_to(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	ObjectType type = static_cast<ObjectType>(LuaConstants::GetConstantFromArg(l, "PhysicsObjectType", 2));

	Body *nearest = Pi::game->GetSpace()->FindNearestTo(b, type);
	LuaObject<Body>::PushToLua(nearest);

	return 1;
}

/*
 * Method: GetPhysRadius
 *
 * Get the body's physical radius
 *
 * > body:GetPhysRadius()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_body_get_phys_radius(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	LuaPush(l, b->GetPhysRadius());
	return 1;
}

static int l_body_get_atmospheric_state(lua_State *l)
{
	Body *planetBody = LuaObject<Body>::CheckFromLua(1);
	Body *b = LuaObject<Body>::CheckFromLua(2);
	//	const SystemBody *sb = b->GetSystemBody();
	vector3d pos = b->GetPositionRelTo(planetBody);
	double center_dist = pos.Length();
	if (planetBody->IsType(ObjectType::PLANET)) {
		double pressure, density;
		static_cast<Planet *>(planetBody)->GetAtmosphericState(center_dist, &pressure, &density);
		lua_pushnumber(l, pressure);
		lua_pushnumber(l, density);
		return 2;
	} else {
		return 0;
	}
}

static int l_body_get_label(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	LuaPush(l, b->GetLabel());
	return 1;
}

static bool push_body_to_lua(Body *body)
{
	assert(body);
	switch (body->GetType()) {
	case ObjectType::BODY:
		LuaObject<Body>::PushToLua(body);
		break;
	case ObjectType::MODELBODY:
		LuaObject<ModelBody>::PushToLua(static_cast<ModelBody *>(body));
		break;
	case ObjectType::SHIP:
		LuaObject<Ship>::PushToLua(static_cast<Ship *>(body));
		break;
	case ObjectType::PLAYER:
		LuaObject<Player>::PushToLua(static_cast<Player *>(body));
		break;
	case ObjectType::SPACESTATION:
		LuaObject<SpaceStation>::PushToLua(static_cast<SpaceStation *>(body));
		break;
	case ObjectType::PLANET:
		LuaObject<Planet>::PushToLua(static_cast<Planet *>(body));
		break;
	case ObjectType::STAR:
		LuaObject<Star>::PushToLua(static_cast<Star *>(body));
		break;
	case ObjectType::CARGOBODY:
		LuaObject<CargoBody>::PushToLua(static_cast<CargoBody *>(body));
		break;
	case ObjectType::MISSILE:
		LuaObject<Missile>::PushToLua(static_cast<Missile *>(body));
		break;
	case ObjectType::HYPERSPACECLOUD:
		LuaObject<HyperspaceCloud>::PushToLua(static_cast<HyperspaceCloud *>(body));
		break;
	default:
		return false;
	}
	return true;
}

static bool pi_lua_body_serializer(lua_State *l, Json &out)
{
	Body *body = LuaObject<Body>::GetFromLua(-1);
	if (!body) return false;
	out = Json(Pi::game->GetSpace()->GetIndexForBody(body));
	return true;
}

static bool pi_lua_body_deserializer(lua_State *l, const Json &obj)
{
	if (!obj.is_number_integer()) return false;
	Body *body = Pi::game->GetSpace()->GetBodyByIndex(obj);
	return push_body_to_lua(body);
}

static int l_body_get_velocity(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	LuaPush<vector3d>(l, b->GetVelocity());
	return 1;
}

static int l_body_set_velocity(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	b->SetVelocity(LuaPull<vector3d>(l, 2));
	return 0;
}

static int l_body_set_ang_velocity(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	b->SetAngVelocity(LuaPull<vector3d>(l, 2));
	return 0;
}

void pi_lua_generic_pull(lua_State *l, int index, ObjectType &objectType)
{
	objectType = s_bodyFlags.LookupEnum(l, index);
}

void pi_lua_generic_push(lua_State *l, ObjectType bodyType)
{
	lua_pushstring(l, EnumStrings::GetString("PhysicsObjectType", static_cast<int>(bodyType)));
}

template <>
const char *LuaObject<Body>::s_type = "Body";

template <>
void LuaObject<Body>::RegisterClass()
{
	const char *l_parent = "PropertiedObject";

	static luaL_Reg l_methods[] = {
		{ "GetComponent", l_body_get_component },
		{ "SetComponent", l_body_set_component },
		{ "IsDynamic", l_body_is_dynamic },
		{ "DistanceTo", l_body_distance_to },
		{ "GetGroundPosition", l_body_get_ground_position },
		{ "FindNearestTo", l_body_find_nearest_to },
		{ "GetVelocityRelTo", l_body_get_velocity_rel_to },
		{ "GetPositionRelTo", l_body_get_position_rel_to },
		{ "GetAltitudeRelTo", l_body_get_altitude_rel_to },
		{ "GetPhysicalRadius", l_body_get_phys_radius },
		{ "GetAtmosphericState", l_body_get_atmospheric_state },
		{ "GetLabel", l_body_get_label },
		{ "IsMoreImportantThan", l_body_is_more_important_than },
		{ "IsMoon", l_body_is_moon },
		{ "IsPlanet", l_body_is_planet },
		{ "IsShip", l_body_is_ship },
		{ "IsHyperspaceCloud", l_body_is_hyperspace_cloud },
		{ "IsMissile", l_body_is_missile },
		{ "IsStation", l_body_is_station },
		{ "IsSpaceStation", l_body_is_space_station },
		{ "IsGroundStation", l_body_is_ground_station },
		{ "IsCargoContainer", l_body_is_cargo_container },
		{ "GetSystemBody", l_body_get_system_body },
		{ "GetVelocity", l_body_get_velocity },
		{ "SetVelocity", l_body_set_velocity },
		{ "SetAngVelocity", l_body_set_ang_velocity },
		{ 0, 0 }
	};

	static luaL_Reg l_attrs[] = {
		{ "seed", l_body_attr_seed },
		{ "path", l_body_attr_path },
		{ "type", l_body_attr_type },
		{ "superType", l_body_attr_super_type },
		{ "frameBody", l_body_attr_frame_body },
		{ "frameRotating", l_body_attr_frame_rotating },
		{ 0, 0 }
	};

	// const SerializerPair body_serializers(_body_serializer, _body_deserializer, _body_to_json, _body_from_json);
	const SerializerPair body_serializers{ pi_lua_body_serializer, pi_lua_body_deserializer };

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Body>::DynamicCastPromotionTest);
	LuaObjectBase::RegisterSerializer(s_type, body_serializers);

	// we're also the serializer for our subclasses
	LuaObjectBase::RegisterSerializer("ModelBody", body_serializers);
	LuaObjectBase::RegisterSerializer("Ship", body_serializers);
	LuaObjectBase::RegisterSerializer("Player", body_serializers);
	LuaObjectBase::RegisterSerializer("SpaceStation", body_serializers);
	LuaObjectBase::RegisterSerializer("Planet", body_serializers);
	LuaObjectBase::RegisterSerializer("Star", body_serializers);
	LuaObjectBase::RegisterSerializer("CargoBody", body_serializers);
	LuaObjectBase::RegisterSerializer("Missile", body_serializers);
	LuaObjectBase::RegisterSerializer("HyperspaceCloud", body_serializers);

	s_bodyFlags.Register(Lua::manager->GetLuaState(), "Constants.ObjectType");
}
