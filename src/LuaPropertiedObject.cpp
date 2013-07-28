// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "PropertiedObject.h"

template <> const char *LuaObject<PropertiedObject>::s_type = "PropertiedObject";

template <> void LuaObject<PropertiedObject>::RegisterClass()
{
	LuaObjectBase::CreateClass(s_type, 0, 0, 0, 0);
}
