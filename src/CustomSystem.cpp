#include "CustomSystem.h"
#include "PiLuaClasses.h"
#include "PiLuaConstants.h"
#include "Polit.h"

static std::list<CustomSystem> custom_systems;

void CustomSystem::Init()
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
    OOLUA::setup_user_lua_state(L);

    PiLuaClasses::RegisterClasses(L);
    PiLuaConstants::RegisterConstants(L);

    OOLUA::register_class<CustomSystem>(L);
    OOLUA::register_class<CustomSBody>(L);

	lua_register(L, "load_lua", mylua_load_lua);

	lua_pushstring(L, PIONEER_DATA_DIR);
	lua_setglobal(L, "CurrentDirectory");

	lua_pushcfunction(L, mylua_panic);
	if (luaL_loadfile(L, (std::string(PIONEER_DATA_DIR) + "/pisystems.lua").c_str())) {
		mylua_panic(L);
	} else {
		lua_pcall(L, 0, LUA_MULTRET, -2);
	}

	lua_close(L);
}

const std::list<const CustomSystem*> CustomSystem::GetCustomSystemsForSector(int sectorX, int sectorY)
{
	std::list<const CustomSystem*> sector_systems;

	for (std::list<CustomSystem>::iterator i = custom_systems.begin(); i != custom_systems.end(); i++) {
		CustomSystem *cs = &(*i);
		if (cs->sectorX == sectorX && cs->sectorY == sectorY)
			sector_systems.push_back(cs);
	}

	return sector_systems;
}

const CustomSystem* CustomSystem::GetCustomSystem(const char *name)
{
	for (std::list<CustomSystem>::iterator i = custom_systems.begin(); i != custom_systems.end(); i++) {
		CustomSystem *cs = &(*i);
		if (!cs->name.compare(name)) return cs;
	}
	return NULL;
}

const SBodyPath CustomSystem::GetSBodyPathForCustomSystem(const CustomSystem* cs)
{
	const std::list<const CustomSystem*> cslist = GetCustomSystemsForSector(cs->sectorX, cs->sectorY);
	int idx = 0;
	for (std::list<const CustomSystem*>::const_iterator i = cslist.begin(); i != cslist.end(); i++) {
		if (!(*i)->name.compare(cs->name)) break;
			idx++;
	}
	assert(idx < static_cast<int>(cslist.size()));
	return SBodyPath(cs->sectorX, cs->sectorY, idx);
}

const SBodyPath CustomSystem::GetSBodyPathForCustomSystem(const char* name)
{
	return GetSBodyPathForCustomSystem(GetCustomSystem(name));
}

CustomSystem::CustomSystem(std::string s, OOLUA::Lua_table t)
{
	name = s;

	for (int i=0 ; i<4; i++) {
		int type;
		if (!t.safe_at(i+1, type))
			type = SBody::TYPE_GRAVPOINT;
        primaryType[i] = static_cast<SBody::BodyType>(type);
	}

	seed = 0;
	govType = Polit::GOV_NONE;
}

void CustomSystem::l_add_to_sector(int x, int y, pi_vector& v)
{
	sectorX = x;
	sectorY = y;
	pos = v;

    custom_systems.push_back(*this);
}

EXPORT_OOLUA_FUNCTIONS_0_CONST(CustomSystem)
EXPORT_OOLUA_FUNCTIONS_6_NON_CONST(CustomSystem, seed, govtype, short_desc, long_desc, primary_star, add_to_sector)

CustomSBody::CustomSBody(std::string s, int t)
{
	name = s;
	type = static_cast<SBody::BodyType>(t);

	averageTemp = 0;
	latitude = longitude = 0.0;
}

EXPORT_OOLUA_FUNCTIONS_0_CONST(CustomSBody)
EXPORT_OOLUA_FUNCTIONS_NON_CONST(CustomSBody, radius, mass, temp, semi_major_axis, eccentricity, latitude, inclination, longitude, rotation_period, axial_tilt, height_map, add)

