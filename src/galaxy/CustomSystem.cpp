#include "CustomSystem.h"
#include "SystemPath.h"

#include "LuaUtils.h"
#include "PiLuaClasses.h"
#include "LuaConstants.h"
#include "Polit.h"
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
	OOLUA::register_class<CustomSystemBody>(L);

	pi_lua_dofile(L, "pistartup.lua");
	pi_lua_dofile_recursive(L, "systems");

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

CustomSystem::CustomSystem(std::string s, OOLUA::Lua_table t)
{
	want_rand_explored = true;

	name = s;

	numStars = 0;

	std::string stype;
	bool done = false;
	for (int i=0 ; i<4; i++) {
		int type = SystemBody::TYPE_GRAVPOINT;
		if (t.safe_at(i+1, stype)) {
			type = LuaConstants::GetConstant(csLua, "BodyType", stype.c_str());
			if ( type < SystemBody::TYPE_STAR_MIN || type > SystemBody::TYPE_STAR_MAX ) {
				printf("system star %d does not have a valid star type\n", i+1);
				assert(0);
			}
		}
		primaryType[i] = static_cast<SystemBody::BodyType>(type);

		if (type == SystemBody::TYPE_GRAVPOINT) done = true;
		if (!done) numStars++;
	}

	seed = 0;
	govType = Polit::GOV_NONE;
}

static void _add_children_to_sbody(lua_State* L, CustomSystemBody* sbody, OOLUA::Lua_table children)
{
	int i=1;
	while (1) {
		CustomSystemBody *kid;

		if (!children.safe_at(i++, kid))
			break;

		if (kid == NULL) {
			luaL_error(L,
				"invalid element (must be CustomSystemBody or table of CustomSystemBody)\n"
				"invalid element is child of CustomSystemBody '%s'", sbody->name.c_str());
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

void CustomSystem::l_bodies(lua_State* L, CustomSystemBody& primary_star, OOLUA::Lua_table children)
{
	if ( primary_star.type < SystemBody::TYPE_STAR_MIN || primary_star.type > SystemBody::TYPE_STAR_MAX )
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

CustomSystemBody::CustomSystemBody(std::string s, std::string stype)
{
	name = s;
	type = static_cast<SystemBody::BodyType>(LuaConstants::GetConstant(csLua, "BodyType", stype.c_str()));

	if ( type < SystemBody::TYPE_MIN || type > SystemBody::TYPE_MAX ) {
		printf("body '%s' does not have a valid type\n", s.c_str());
		assert(0);
	}

	seed = averageTemp = 0;
	latitude = longitude = 0.0;
	want_rand_offset = true;
	want_rand_seed = true;
}

CustomSystemBody* CustomSystemBody::l_height_map(lua_State *L, std::string f, unsigned int n) {
	heightMapFilename = FileSystem::JoinPathBelow("heightmaps", f);
	heightMapFractal = n;
	if (n >= 2) luaL_error(L, "invalid terrain fractal type");
		return this; 
}

EXPORT_OOLUA_FUNCTIONS_0_CONST(CustomSystemBody)

// this is the same as EXPORT_OOLUA_FUNCTIONS_*_NON_CONST. oolua doesn't
// provide a macro for that many members, and the varargs version seems to
// fail after 16 parameters
CLASS_LIST_MEMBERS_START_OOLUA_NON_CONST(CustomSystemBody)
LUA_MEMBER_FUNC_9(OOLUA::Proxy_class<CustomSystemBody>, seed, radius, mass, temp, semi_major_axis, eccentricity, orbital_offset, latitude, inclination)
LUA_MEMBER_FUNC_9(OOLUA::Proxy_class<CustomSystemBody>, longitude, rotation_period, axial_tilt, height_map, metallicity, volcanicity, atmos_density, atmos_oxidizing, ocean_cover)
LUA_MEMBER_FUNC_2(OOLUA::Proxy_class<CustomSystemBody>, ice_cover, life)
CLASS_LIST_MEMBERS_END
