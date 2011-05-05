#include "LuaBody.h"
#include "LuaSBodyPath.h"
#include "LuaUtils.h"
#include "Body.h"
#include "StarSystem.h"
#include "Pi.h"

/*
 * Class: Body
 *
 * Class representing that represents a physics body.
 */

/*
 * Attribute: label
 */
static int l_body_attr_label(lua_State *l)
{
	Body *b = LuaBody::GetFromLua(1);
	lua_pushstring(l, b->GetLabel().c_str());
	return 1;
} 

/*
 * Attribute: seed
 */
static int l_body_attr_seed(lua_State *l)
{
	Body *b = LuaBody::GetFromLua(1);

	const SBody *sbody = b->GetSBody();
	assert(sbody);

	lua_pushinteger(l, sbody->seed);
	return 1;
}

/*
 * Attribute: path
 */
static int l_body_attr_path(lua_State *l)
{
	Body *b = LuaBody::GetFromLua(1);

	const SBody *sbody = b->GetSBody();
	if (!sbody) {
		lua_pushnil(l);
		return 1;
	}

	SBodyPath *sbp = new SBodyPath(Pi::currentSystem->SectorX(), Pi::currentSystem->SectorY(), Pi::currentSystem->SystemIdx());
	sbp->sbodyId = sbody->id;

	LuaSBodyPath::PushToLuaGC(sbp);

	return 1;
}

template <> const char *LuaObject<Body>::s_type = "Body";

template <> void LuaObject<Body>::RegisterClass()
{
	static luaL_reg l_attrs[] = {
		{ "label", l_body_attr_label },
		{ "seed",  l_body_attr_seed  },
		{ "path",  l_body_attr_path  },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, NULL, l_attrs, NULL);
}
