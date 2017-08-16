// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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
#include "LuaPiGui.h"

#include "ModelBody.h"
#include "Ship.h"
#include "Player.h"
#include "SpaceStation.h"
#include "Planet.h"
#include "Star.h"
#include "CargoBody.h"
#include "Missile.h"

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
		if(!sb) {
			LuaPush<bool>(l, false);
		} else {
			LuaPush<bool>(l, sb->IsMoon());
		}
		return 1;
}

static int l_body_is_missile(lua_State *l)
{
		Body *body = LuaObject<Body>::CheckFromLua(1);
		LuaPush<bool>(l, body->GetType() == Object::Type::MISSILE);
		return 1;
}

static int l_body_is_cargo_container(lua_State *l)
{
		Body *body = LuaObject<Body>::CheckFromLua(1);
		LuaPush<bool>(l, body->GetType() == Object::Type::CARGOBODY);
		return 1;
}

static int l_body_is_ship(lua_State *l)
{
		Body *body = LuaObject<Body>::CheckFromLua(1);
		LuaPush<bool>(l, body->GetType() == Object::Type::SHIP);
		return 1;
}

static int l_body_is_hyperspace_cloud(lua_State *l)
{
		Body *body = LuaObject<Body>::CheckFromLua(1);
		LuaPush<bool>(l, body->GetType() == Object::Type::HYPERSPACECLOUD);
		return 1;
}

static int l_body_is_planet(lua_State *l)
{
		Body *body = LuaObject<Body>::CheckFromLua(1);
		const SystemBody *sb = body->GetSystemBody();
		if(!sb) {
			LuaPush<bool>(l, false);
		} else {
			LuaPush<bool>(l, sb->IsPlanet());
		}
		return 1;
}

static int l_body_get_system_body(lua_State *l)
{
		Body *body = LuaObject<Body>::CheckFromLua(1);
		SystemBody *sb = const_cast<SystemBody*>(body->GetSystemBody()); // TODO: ugly, change this...
		if(!sb) {
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

	if(body == other)
	{
		LuaPush<bool>(l, false);
		return 1;
	}

	Object::Type a = body->GetType();
	const SystemBody *sb_a = body->GetSystemBody();
	bool a_gas_giant = sb_a && sb_a->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT;
	bool a_planet = sb_a && sb_a->IsPlanet();
	bool a_moon = sb_a && sb_a->IsMoon();

	Object::Type b = other->GetType();
	const SystemBody *sb_b = other->GetSystemBody();
	bool b_gas_giant = sb_b && sb_b->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT;
	bool b_planet = sb_b && sb_b->IsPlanet();
	bool b_moon = sb_b && sb_b->IsMoon();

	bool result = false;

	// if type is the same, just sort alphabetically
	// planets are different, because moons are
	// less important (but don't have their own type)
	if(a == b && a != Object::Type::PLANET) result = body->GetLabel() < other->GetLabel();
	// a star is larger than any other object
	else if(a == Object::Type::STAR) result = true;
	// any (non-star) object is smaller than a star
	else if(b == Object::Type::STAR) result = false;
	// a gas giant is larger than anything but a star,
	// but remember to keep total order in mind: if both are
	// gas giants, order alphabetically
	else if(a_gas_giant) result = !b_gas_giant || body->GetLabel() < other->GetLabel();
	// any (non-star, non-gas giant) object is smaller than a gas giant
	else if(b_gas_giant) result = false;
	// between two planets or moons, alphabetic
	else if(a_planet && b_planet) result = body->GetLabel() < other->GetLabel();
	else if(a_moon && b_moon) result = body->GetLabel() < other->GetLabel();
	// a planet is larger than any non-planet
	else if(a_planet) result = true;
	// a non-planet is smaller than any planet
	else if(b_planet) result = false;
	// a moon is larger than any non-moon
	else if(a_moon) result = true;
	// a non-moon is smaller than any moon
	else if(b_moon) result = false;
	// spacestation > city > ship > hyperspace cloud > cargo body > missile > projectile
	else if(a == Object::Type::SPACESTATION) result = true;
	else if(b == Object::Type::SPACESTATION) result = false;
    else if(a == Object::Type::CITYONPLANET) result = true;
    else if(b == Object::Type::CITYONPLANET) result = false;
	else if(a == Object::Type::SHIP) result = true;
	else if(b == Object::Type::SHIP) result = false;
	else if(a == Object::Type::HYPERSPACECLOUD) result = true;
	else if(b == Object::Type::HYPERSPACECLOUD) result = false;
	else if(a == Object::Type::CARGOBODY) result = true;
	else if(b == Object::Type::CARGOBODY) result = false;
	else if(a == Object::Type::MISSILE) result = true;
	else if(b == Object::Type::MISSILE) result = false;
	else if(a == Object::Type::PROJECTILE) result = true;
	else if(b == Object::Type::PROJECTILE) result = false;
	else Error("don't know how to compare %i and %i\n", a, b);

	LuaPush<bool>(l, result);
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
	LuaPush(l, velocity);
	return 1;
}

/*
 * Method: GetAltitudeRelTo
 *
 * Get the body's altitude relative to another body
 *
 * > body:GetAltitudeRelTo(otherBody)
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

static int l_body_get_altitude_rel_to(lua_State *l)
{
	const Body *other = LuaObject<Body>::CheckFromLua(2);
	vector3d pos = Pi::player->GetPositionRelTo(other);
	double center_dist = pos.Length();
	if(other && other->IsType(Object::TERRAINBODY)) {
		const TerrainBody* terrain = static_cast<const TerrainBody*>(other);
		vector3d surface_pos = pos.Normalized();
		double radius = 0.0;
		if (center_dist <= 3.0 * terrain->GetMaxFeatureRadius()) {
			radius = terrain->GetTerrainHeight(surface_pos);
		}
		double altitude = center_dist - radius;
		if (altitude < 0)
			altitude = 0;
		LuaPush(l, altitude);
		return 1;
	} else {
		LuaPush(l, center_dist);
		return 1;
	}

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

/*
 * Method: GetProjectedScreenPosition
 *
 * Get the body's position projected to screen space as a Vector
 *
 * > body:GetProjectedScreenPosition()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_body_get_projected_screen_position(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	WorldView *wv = Pi::game->GetWorldView();
	vector3d p = wv->WorldSpaceToScreenSpace(b);
	return pushOnScreenPositionDirection(l, p);
}

static int l_body_get_atmospheric_state(lua_State *l) {
	Body *b = LuaObject<Body>::CheckFromLua(1);
	//	const SystemBody *sb = b->GetSystemBody();
	vector3d pos = Pi::player->GetPosition();
	double center_dist = pos.Length();
	if (b->IsType(Object::PLANET)) {
		double pressure, density;
		static_cast<Planet*>(b)->GetAtmosphericState(center_dist, &pressure, &density);
		lua_pushnumber(l, pressure);
		lua_pushnumber(l, density);
		return 2;
	} else {
		return 0;
	}
}

static int l_body_get_target_indicator_screen_position(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	WorldView *wv = Pi::game->GetWorldView();
	vector3d p = wv->GetTargetIndicatorScreenPosition(b);
	return pushOnScreenPositionDirection(l, p);
}

static std::string _body_serializer(LuaWrappable *o)
{
	static char buf[256];
	Body *b = static_cast<Body*>(o);
	snprintf(buf, sizeof(buf), "%u\n", Pi::game->GetSpace()->GetIndexForBody(b));
	return std::string(buf);
}

static bool _body_deserializer(const char *pos, const char **next)
{
	Uint32 n = strtoul(pos, const_cast<char**>(next), 0);
	if (pos == *next) return false;
	(*next)++; // skip newline

	Body *body = Pi::game->GetSpace()->GetBodyByIndex(n);

	switch (body->GetType()) {
	case Object::BODY:
		LuaObject<Body>::PushToLua(body);
		break;
	case Object::MODELBODY:
		LuaObject<Body>::PushToLua(dynamic_cast<ModelBody*>(body));
		break;
	case Object::SHIP:
		LuaObject<Ship>::PushToLua(dynamic_cast<Ship*>(body));
		break;
	case Object::PLAYER:
		LuaObject<Player>::PushToLua(dynamic_cast<Player*>(body));
		break;
	case Object::SPACESTATION:
		LuaObject<SpaceStation>::PushToLua(dynamic_cast<SpaceStation*>(body));
		break;
	case Object::PLANET:
		LuaObject<Planet>::PushToLua(dynamic_cast<Planet*>(body));
		break;
	case Object::STAR:
		LuaObject<Star>::PushToLua(dynamic_cast<Star*>(body));
		break;
	case Object::CARGOBODY:
		LuaObject<Star>::PushToLua(dynamic_cast<CargoBody*>(body));
		break;
	case Object::MISSILE:
		LuaObject<Missile>::PushToLua(dynamic_cast<Missile*>(body));
		break;
	default:
		return false;
	}

	return true;
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
		{ "GetVelocityRelTo",  l_body_get_velocity_rel_to },
		{ "GetPositionRelTo",  l_body_get_position_rel_to },
		{ "GetAltitudeRelTo",  l_body_get_altitude_rel_to },
		{ "GetProjectedScreenPosition", l_body_get_projected_screen_position },
		{ "GetTargetIndicatorScreenPosition", l_body_get_target_indicator_screen_position },
		{ "GetPhysicalRadius",   l_body_get_phys_radius },
		{ "GetAtmosphericState", l_body_get_atmospheric_state },
		{ "IsMoreImportantThan", l_body_is_more_important_than },
		{ "IsMoon",              l_body_is_moon },
		{ "IsPlanet",            l_body_is_planet },
		{ "IsShip",              l_body_is_ship },
		{ "IsHyperspaceCloud",   l_body_is_hyperspace_cloud },
		{ "IsMissile",           l_body_is_missile },
		{ "IsCargoContainer",    l_body_is_cargo_container },
		{ "GetSystemBody",       l_body_get_system_body },
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
	LuaObjectBase::RegisterSerializer(s_type, SerializerPair(_body_serializer, _body_deserializer));

	// we're also the serializer for our subclasses
	LuaObjectBase::RegisterSerializer("ModelBody",    SerializerPair(_body_serializer, _body_deserializer));
	LuaObjectBase::RegisterSerializer("Ship",         SerializerPair(_body_serializer, _body_deserializer));
	LuaObjectBase::RegisterSerializer("Player",       SerializerPair(_body_serializer, _body_deserializer));
	LuaObjectBase::RegisterSerializer("SpaceStation", SerializerPair(_body_serializer, _body_deserializer));
	LuaObjectBase::RegisterSerializer("Planet",       SerializerPair(_body_serializer, _body_deserializer));
	LuaObjectBase::RegisterSerializer("Star",         SerializerPair(_body_serializer, _body_deserializer));
	LuaObjectBase::RegisterSerializer("CargoBody",    SerializerPair(_body_serializer, _body_deserializer));
	LuaObjectBase::RegisterSerializer("Missile",      SerializerPair(_body_serializer, _body_deserializer));
}
