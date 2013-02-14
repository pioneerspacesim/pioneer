// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUANAMEGEN_H
#define _LUANAMEGEN_H

#include <string>

class LuaManager;
class Random;
class SystemBody;

class LuaNameGen {
public:
	LuaNameGen(LuaManager *manager): m_luaManager(manager) {}

	std::string FullName(bool isFemale, Random &rng);
	std::string Surname(Random &rng);
	std::string BodyName(SystemBody *body, Random &rng);

private:
	LuaManager *m_luaManager;
};

#endif
