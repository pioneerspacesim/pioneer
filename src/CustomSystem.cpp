#include "CustomSystem.h"
#include "LuaUtils.h"
#include "PiLuaClasses.h"
#include "LuaConstants.h"
#include "Polit.h"

static lua_State *csLua;

static std::list<CustomSystem> custom_systems;

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

	lua_pushstring(L, PIONEER_DATA_DIR);
	lua_setglobal(L, "CurrentDirectory");

	lua_pushcfunction(L, pi_lua_panic);
	if (luaL_loadfile(L, (std::string(PIONEER_DATA_DIR) + "/pisystems.lua").c_str())) {
		pi_lua_panic(L);
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
	if ( primary_star.type < SBody::TYPE_BROWN_DWARF || primary_star.type > SBody::TYPE_WHITE_DWARF )
		luaL_error(L, "first body does not have a valid star type");
	if ( primary_star.type != primaryType[0] )
		luaL_error(L, "first body is not of same type as system primary star");

	_add_children_to_sbody(L, &primary_star, children);
	sBody = primary_star;
}

void CustomSystem::l_add_to_sector(int x, int y, pi_vector& v)
{
	sectorX = x;
	sectorY = y;
	pos = v;

	custom_systems.push_back(*this);
}

EXPORT_OOLUA_FUNCTIONS_0_CONST(CustomSystem)
EXPORT_OOLUA_FUNCTIONS_6_NON_CONST(CustomSystem, seed, govtype, short_desc, long_desc, bodies, add_to_sector)

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

EXPORT_OOLUA_FUNCTIONS_0_CONST(CustomSBody)

// this is the same as EXPORT_OOLUA_FUNCTIONS_*_NON_CONST. oolua doesn't
// provide a macro for that many members, and the varargs version seems to
// fail after 16 parameters
CLASS_LIST_MEMBERS_START_OOLUA_NON_CONST(CustomSBody)
LUA_MEMBER_FUNC_9(OOLUA::Proxy_class<CustomSBody>, seed, radius, mass, temp, semi_major_axis, eccentricity, orbital_offset, latitude, inclination)
LUA_MEMBER_FUNC_9(OOLUA::Proxy_class<CustomSBody>, longitude, rotation_period, axial_tilt, height_map, metallicity, volcanicity, atmos_density, atmos_oxidizing, ocean_cover)
LUA_MEMBER_FUNC_2(OOLUA::Proxy_class<CustomSBody>, ice_cover, life)
CLASS_LIST_MEMBERS_END
