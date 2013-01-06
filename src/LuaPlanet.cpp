// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaPlanet.h"
#include "LuaUtils.h"

/*
 * Class: Planet
 *
 * Class representing a planet. Inherits from <Body>.
 */

template <> const char *LuaObject<Planet>::s_type = "Planet";

template <> void LuaObject<Planet>::RegisterClass()
{
	const char *l_parent = "Body";

	LuaObjectBase::CreateClass(s_type, l_parent, NULL, NULL, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Planet>::DynamicCastPromotionTest);
}
