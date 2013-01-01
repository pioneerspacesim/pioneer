// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUASTARSYSTEM_H
#define _LUASTARSYSTEM_H

#include "LuaObject.h"
#include "galaxy/StarSystem.h"

template <> class LuaAcquirer<StarSystem> : public LuaAcquirerRefCounted {};
typedef LuaObject<StarSystem> LuaStarSystem;

#endif
