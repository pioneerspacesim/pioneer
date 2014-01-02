// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPERTIEDOBJECT_H
#define PROPERTIEDOBJECT_H

#include "PropertyMap.h"

class LuaManager;

class PropertiedObject {
public:
	PropertyMap &Properties() { return m_properties; }

protected:
	PropertiedObject(LuaManager *lua) : m_properties(lua) {}

private:
	PropertyMap m_properties;
};

#endif
