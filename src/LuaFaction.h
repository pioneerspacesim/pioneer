// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAFaction_H
#define _LUAFaction_H

#include "LuaObject.h"
#include "Factions.h"

//typedef LuaObject<Faction> LuaFaction;
class Faction;
typedef LuaObjectUncopyable<Faction,LuaUncopyable<Faction> > LuaFaction;

#endif
