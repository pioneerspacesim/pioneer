// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUANAMEGEN_H
#define _LUANAMEGEN_H

#include "RefCounted.h"
#include <string>

class LuaManager;
class Random;
class SystemBody;

class LuaNameGen {
public:
	LuaNameGen(LuaManager *manager) :
		m_luaManager(manager) {}

	std::string FullName(bool isFemale, RefCountedPtr<Random> &rng);
	std::string Surname(RefCountedPtr<Random> &rng);
	std::string BodyName(SystemBody *body, RefCountedPtr<Random> &rng);

private:
	LuaManager *m_luaManager;
};

#endif
