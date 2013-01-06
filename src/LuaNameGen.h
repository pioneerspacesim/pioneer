// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUANAMEGEN_H
#define _LUANAMEGEN_H

#include <string>

class LuaManager;
class MTRand;
class SystemBody;

class LuaNameGen {
public:
	LuaNameGen(LuaManager *manager): m_luaManager(manager) {}

	std::string FullName(bool isFemale, MTRand &rng);
	std::string Surname(MTRand &rng);
	std::string BodyName(SystemBody *body, MTRand &rng);

private:
	LuaManager *m_luaManager;
};

#endif
