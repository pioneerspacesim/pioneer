// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CargoBody.h"
#include "LuaObject.h"
#include "LuaUtils.h"

/*
 * Class: CargoBody
 *
 * Class representing an item of cargo floating in space. Inherits from
 * <ModelBody>.
 */

template <>
const char *LuaObject<CargoBody>::s_type = "CargoBody";

template <>
void LuaObject<CargoBody>::RegisterClass()
{
	const char *l_parent = "ModelBody";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<CargoBody>::DynamicCastPromotionTest);
}
