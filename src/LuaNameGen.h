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
