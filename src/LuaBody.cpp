// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EnumStrings.h"
#include "Body.h"
#include "galaxy/StarSystem.h"
#include "Frame.h"
#include "TerrainBody.h"
#include "Pi.h"
#include "Game.h"

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

	lua_pushinteger(l, sbody->GetSeed());
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
	if (!b->IsType(Object::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = b->GetFrame();
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
	if (!b->IsType(Object::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = b->GetFrame();
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
	lua_pushboolean(l, b->IsType(Object::DYNAMICBODY));
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
 * Get latitude, longitude and altitude of a dynamic body close to the ground or nil the body is not a dynamic body
 * or is not close to the ground.
 *
 * > latitude, longitude, altitude = body:GetGroundPosition()
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
	if (!b->IsType(Object::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = b->GetFrame();
	if (!f->IsRotFrame())
		return 0;

	vector3d pos = b->GetPosition();
	double latitude = atan2(pos.y, sqrt(pos.x*pos.x + pos.z * pos.z));
	double longitude = atan2(pos.x, pos.z);
	lua_pushnumber(l, latitude);
	lua_pushnumber(l, longitude);
	Body *astro = f->GetBody();
	if (astro->IsType(Object::TERRAINBODY)) {
		double radius = static_cast<TerrainBody*>(astro)->GetTerrainHeight(pos.Normalized());
		double altitude = pos.Length() - radius;
		lua_pushnumber(l, altitude);
	} else {
		lua_pushnil(l);
	}
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
	Object::Type type = static_cast<Object::Type>(LuaConstants::GetConstantFromArg(l, "PhysicsObjectType", 2));

	Body *near = Pi::game->GetSpace()->FindNearestTo(b, type);
	LuaObject<Body>::PushToLua(near);

	return 1;
}

template <> const char *LuaObject<Body>::s_type = "Body";

template <> void LuaObject<Body>::RegisterClass()
{
	const char *l_parent = "PropertiedObject";

	static luaL_Reg l_methods[] = {
		{ "IsDynamic",  l_body_is_dynamic  },
		{ "DistanceTo", l_body_distance_to },
		{ "GetGroundPosition", l_body_get_ground_position },
		{ "FindNearestTo", l_body_find_nearest_to },
		{ 0, 0 }
	};

	static luaL_Reg l_attrs[] = {
		{ "seed",          l_body_attr_seed           },
		{ "path",          l_body_attr_path           },
		{ "type",          l_body_attr_type           },
		{ "superType",     l_body_attr_super_type     },
		{ "frameBody",     l_body_attr_frame_body     },
		{ "frameRotating", l_body_attr_frame_rotating },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Body>::DynamicCastPromotionTest);
}
