// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPERTIEDOBJECT_H
#define PROPERTIEDOBJECT_H

#include "core/Property.h"

class LuaManager;

class PropertiedObject {
public:
	PropertyMap &Properties() { return m_properties; }
	const PropertyMap &Properties() const { return m_properties; }

private:
	PropertyMap m_properties;
};

#endif
