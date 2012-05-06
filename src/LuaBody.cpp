#include "LuaBody.h"
#include "LuaSystemPath.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Body.h"
#include "galaxy/StarSystem.h"
#include "Pi.h"
#include "Frame.h"

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
static int l_body_attr_label(lua_State *l)
{
	Body *b = LuaBody::GetFromLua(1);
	lua_pushstring(l, b->GetLabel().c_str());
	return 1;
} 

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
	Body *b = LuaBody::GetFromLua(1);

	const SystemBody *sbody = b->GetSystemBody();
	assert(sbody);

	lua_pushinteger(l, sbody->seed);
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
	Body *b = LuaBody::GetFromLua(1);

	const SystemBody *sbody = b->GetSystemBody();
	if (!sbody) {
		lua_pushnil(l);
		return 1;
	}

	SystemPath path = sbody->path;
	LuaSystemPath::PushToLua(&path);

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
	Body *b = LuaBody::GetFromLua(1);
	const SystemBody *sbody = b->GetSystemBody();
	if (!sbody) {
		lua_pushnil(l);
		return 1;
	}

	lua_pushstring(l, LuaConstants::GetConstantString(l, "BodyType", sbody->type));
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
	Body *b = LuaBody::GetFromLua(1);
	const SystemBody *sbody = b->GetSystemBody();
	if (!sbody) {
		lua_pushnil(l);
		return 1;
	}

	lua_pushstring(l, LuaConstants::GetConstantString(l, "BodySuperType", sbody->GetSuperType()));
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
	Body *b = LuaBody::GetFromLua(1);
	if (!b->IsType(Object::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = b->GetFrame();
	LuaBody::PushToLua(f->GetBodyFor());
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
	Body *b = LuaBody::GetFromLua(1);
	if (!b->IsType(Object::DYNAMICBODY)) {
		lua_pushnil(l);
		return 1;
	}

	Frame *f = b->GetFrame();
	lua_pushboolean(l, f->IsRotatingFrame());
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
	Body *b = LuaBody::GetFromLua(1);
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
	Body *b1 = LuaBody::GetFromLua(1);
	Body *b2 = LuaBody::GetFromLua(2);
	lua_pushnumber(l, b1->GetPositionRelTo(b2).Length());
	return 1;
}

template <> const char *LuaObject<Body>::s_type = "Body";

template <> void LuaObject<Body>::RegisterClass()
{
	static luaL_reg l_methods[] = {
		{ "IsDynamic",  l_body_is_dynamic  },
		{ "DistanceTo", l_body_distance_to },
		{ 0, 0 }
	};

	static luaL_reg l_attrs[] = {
		{ "label",         l_body_attr_label          },
		{ "seed",          l_body_attr_seed           },
		{ "path",          l_body_attr_path           },
		{ "type",          l_body_attr_type           },
		{ "superType",     l_body_attr_super_type     },
		{ "frameBody",     l_body_attr_frame_body     },
		{ "frameRotating", l_body_attr_frame_rotating },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_attrs, NULL);
}
