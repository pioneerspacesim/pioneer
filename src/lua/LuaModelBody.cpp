// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "ModelBody.h"
#include "scenegraph/Model.h"

/*
 * Class: ModelBody
 *
 * Class representing a body with an attached model. Inherits from <Body>.
 */

class LuaModelBody {
public:
	static int l_attr_model(lua_State *l)
	{
		ModelBody *mb = LuaObject<ModelBody>::CheckFromLua(1);
		LuaObject<SceneGraph::Model>::PushToLua(mb->GetModel());
		return 1;
	}
};

template <>
const char *LuaObject<ModelBody>::s_type = "ModelBody";

template <>
void LuaObject<ModelBody>::RegisterClass()
{
	const char *l_parent = "Body";

	static luaL_Reg l_attrs[] = {
		{ "model", LuaModelBody::l_attr_model },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, 0, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<ModelBody>::DynamicCastPromotionTest);
}
