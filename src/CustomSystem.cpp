#include "CustomSystem.h"
#include "LuaUtils.h"
#include "PiLuaClasses.h"
#include "LuaConstants.h"
#include "Polit.h"
#include "SystemPath.h"
#include "FileSystem.h"
#include <map>

typedef std::list<CustomSystem> SystemList;
typedef std::map<SystemPath,SystemList> SectorMap;

static lua_State *csLua;

static SectorMap sector_map;

void CustomSystem::Init()
{
	lua_State *L = csLua = lua_open();
	luaL_openlibs(L);
	OOLUA::setup_user_lua_state(L);

	PiLuaClasses::RegisterClasses(L);
	LuaConstants::Register(L);

	OOLUA::register_class<CustomSystem>(L);
	OOLUA::register_class<CustomSBody>(L);

	lua_register(L, "load_lua", pi_load_lua);

	pi_lua_dofile(L, "pisystems.lua");

	lua_close(L);
}

const std::list<const CustomSystem*> CustomSystem::GetCustomSystemsForSector(int x, int y, int z)
{
	SystemPath path(x,y,z);

	SectorMap::iterator map_i = sector_map.find(path);

	std::list<const CustomSystem*> sector_systems;
	if (map_i != sector_map.end()) {
		for (SystemList::iterator i = (*map_i).second.begin(); i != (*map_i).second.end(); ++i) {
			CustomSystem *cs = &(*i);
			sector_systems.push_back(cs);
		}
	}

	return sector_systems;
}

const CustomSystem* CustomSystem::GetCustomSystem(const char *name)
{
	for (SectorMap::iterator map_i = sector_map.begin(); map_i != sector_map.end(); ++map_i) {
		for (SystemList::iterator i = (*map_i).second.begin(); i != (*map_i).second.end(); ++i) {
			CustomSystem *cs = &(*i);
			if (cs->name != name) return cs;
		}
	}
	return NULL;
}

const SystemPath CustomSystem::GetPathForCustomSystem(const CustomSystem* cs)
{
	const std::list<const CustomSystem*> cslist = GetCustomSystemsForSector(cs->sectorX, cs->sectorY, cs->sectorZ);
	int idx = 0;
	for (std::list<const CustomSystem*>::const_iterator i = cslist.begin(); i != cslist.end(); ++i) {
		if (!(*i)->name.compare(cs->name)) break;
		idx++;
	}
	assert(idx < static_cast<int>(cslist.size()));
	return SystemPath(cs->sectorX, cs->sectorY, cs->sectorZ, idx);
}

const SystemPath CustomSystem::GetPathForCustomSystem(const char* name)
{
	return GetPathForCustomSystem(GetCustomSystem(name));
}

CustomSystem::CustomSystem(std::string s, OOLUA::Lua_table t)
{
	want_rand_explored = true;

	name = s;

	numStars = 0;

	std::string stype;
	bool done = false;
	for (int i=0 ; i<4; i++) {
		int type = SBody::TYPE_GRAVPOINT;
		if (t.safe_at(i+1, stype)) {
			type = LuaConstants::GetConstant(csLua, "BodyType", stype.c_str());
			if ( type < SBody::TYPE_STAR_MIN || type > SBody::TYPE_STAR_MAX ) {
				printf("system star %d does not have a valid star type\n", i+1);
				assert(0);
			}
		}
		primaryType[i] = static_cast<SBody::BodyType>(type);

		if (type == SBody::TYPE_GRAVPOINT) done = true;
		if (!done) numStars++;
	}

	seed = 0;
	govType = Polit::GOV_NONE;
}

static void _add_children_to_sbody(lua_State* L, CustomSBody* sbody, OOLUA::Lua_table children)
{
	int i=1;
	while (1) {
		CustomSBody *kid;

		if (!children.safe_at(i++, kid))
			break;

		if (kid == NULL) {
			luaL_error(L,
				"invalid element (must be CustomSBody or table of CustomSBody)\n"
				"invalid element is child of CustomSBody '%s'", sbody->name.c_str());
		}

		while (1) {
			OOLUA::Lua_table sub;

			if (!children.safe_at(i, sub))
				break;

			if (!sub.valid())
				break;

			_add_children_to_sbody(L, kid, sub);
			i++;
			continue;
		}

		sbody->children.push_back(*kid);
	}
}

CustomSystem *CustomSystem::l_govtype(std::string st)
{
	govType = static_cast<Polit::GovType>(LuaConstants::GetConstant(csLua, "PolitGovType", st.c_str()));
	return this;
}

void CustomSystem::l_bodies(lua_State* L, CustomSBody& primary_star, OOLUA::Lua_table children)
{
	if ( primary_star.type < SBody::TYPE_STAR_MIN || primary_star.type > SBody::TYPE_STAR_MAX )
		luaL_error(L, "first body does not have a valid star type");
	if ( primary_star.type != primaryType[0] )
		luaL_error(L, "first body is not of same type as system primary star");

	_add_children_to_sbody(L, &primary_star, children);
	sBody = primary_star;
}

void CustomSystem::l_add_to_sector(int x, int y, int z, pi_vector& v)
{
	SystemPath path(x,y,z);
	sectorX = x;
	sectorY = y;
	sectorZ = z;
	pos = v;

	sector_map[path].push_back(*this);
}

EXPORT_OOLUA_FUNCTIONS_0_CONST(CustomSystem)
EXPORT_OOLUA_FUNCTIONS_7_NON_CONST(CustomSystem, seed, explored, govtype, short_desc, long_desc, bodies, add_to_sector)

CustomSBody::CustomSBody(std::string s, std::string stype)
{
	name = s;
	type = static_cast<SBody::BodyType>(LuaConstants::GetConstant(csLua, "BodyType", stype.c_str()));

	if ( type < SBody::TYPE_MIN || type > SBody::TYPE_MAX ) {
		printf("body '%s' does not have a valid type\n", s.c_str());
		assert(0);
	}

	seed = averageTemp = 0;
	latitude = longitude = 0.0;
	want_rand_offset = true;
	want_rand_seed = true;
}

CustomSBody* CustomSBody::l_height_map(lua_State *L, std::string f, unsigned int n) {
	heightMapFilename = FileSystem::JoinPathBelow("heightmaps", f);
	heightMapFractal = n;
	if (n >= 2) luaL_error(L, "invalid terrain fractal type");
		return this; 
}

EXPORT_OOLUA_FUNCTIONS_0_CONST(CustomSBody)

// this is the same as EXPORT_OOLUA_FUNCTIONS_*_NON_CONST. oolua doesn't
// provide a macro for that many members, and the varargs version seems to
// fail after 16 parameters
CLASS_LIST_MEMBERS_START_OOLUA_NON_CONST(CustomSBody)
LUA_MEMBER_FUNC_9(OOLUA::Proxy_class<CustomSBody>, seed, radius, mass, temp, semi_major_axis, eccentricity, orbital_offset, latitude, inclination)
LUA_MEMBER_FUNC_9(OOLUA::Proxy_class<CustomSBody>, longitude, rotation_period, axial_tilt, height_map, metallicity, volcanicity, atmos_density, atmos_oxidizing, ocean_cover)
LUA_MEMBER_FUNC_2(OOLUA::Proxy_class<CustomSBody>, ice_cover, life)
CLASS_LIST_MEMBERS_END
