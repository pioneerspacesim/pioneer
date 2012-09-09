#include "CustomSystem.h"
#include "SystemPath.h"

#include "LuaUtils.h"
#include "LuaVector.h"
#include "LuaFixed.h"
#include "LuaConstants.h"
#include "Polit.h"
#include "FileSystem.h"
#include <map>

typedef std::map<SystemPath, CustomSystem::SystemList> SectorMap;

static SectorMap s_sectorMap;
static const CustomSystem::SystemList s_emptySystemList; // see: Null Object pattern

// ------- CustomSystemBody --------

static const char LuaCustomSystemBody_TypeName[] = "CustomSystemBody";

static CustomSystemBody **l_csb_check_ptr(lua_State *L, int idx) {
	CustomSystemBody **csbptr = static_cast<CustomSystemBody**>(
			luaL_checkudata(L, idx, LuaCustomSystemBody_TypeName));
	if (!(*csbptr)) {
		abort();
		luaL_argerror(L, idx, "invalid body (this body has already been used)");
	}
	return csbptr;
}

static CustomSystemBody *l_csb_check(lua_State *L, int idx)
{ return *l_csb_check_ptr(L, idx); }

static int l_csb_new(lua_State *L)
{
	const char *name = luaL_checkstring(L, 2);
	int type = LuaConstants::GetConstantFromArg(L, "BodyType", 3);

	if (type < SystemBody::TYPE_MIN || type > SystemBody::TYPE_MAX) {
		return luaL_error(L, "body '%s' does not have a valid type", name);
	}

	CustomSystemBody **csbptr = static_cast<CustomSystemBody**>(
			lua_newuserdata(L, sizeof(CustomSystemBody*)));
	*csbptr = new CustomSystemBody;
	luaL_setmetatable(L, LuaCustomSystemBody_TypeName);

	(*csbptr)->name = name;
	(*csbptr)->type = static_cast<SystemBody::BodyType>(type);

	return 1;
}

#define CSB_FIELD_SETTER_FIXED(luaname, fieldname)         \
	static int l_csb_ ## luaname (lua_State *L) {          \
		CustomSystemBody *csb = l_csb_check(L, 1);         \
		const fixed *value = LuaFixed::CheckFromLua(L, 2); \
		csb->fieldname = *value;                         \
		lua_settop(L, 1); return 1;                        \
	}

#define CSB_FIELD_SETTER_FLOAT(luaname, fieldname)         \
	static int l_csb_ ## luaname (lua_State *L) {          \
		CustomSystemBody *csb = l_csb_check(L, 1);         \
		double value = luaL_checknumber(L, 2);             \
		csb->fieldname = value;                          \
		lua_settop(L, 1); return 1;                        \
	}

#define CSB_FIELD_SETTER_INT(luaname, fieldname)           \
	static int l_csb_ ## luaname (lua_State *L) {          \
		CustomSystemBody *csb = l_csb_check(L, 1);         \
		int value = luaL_checkinteger(L, 2);               \
		csb->fieldname = value;                          \
		lua_settop(L, 1); return 1;                        \
	}

CSB_FIELD_SETTER_FIXED(radius, radius)
CSB_FIELD_SETTER_FIXED(mass, mass)
CSB_FIELD_SETTER_INT(temp, averageTemp)
CSB_FIELD_SETTER_FIXED(semi_major_axis, semiMajorAxis)
CSB_FIELD_SETTER_FIXED(eccentricity, eccentricity)
CSB_FIELD_SETTER_FLOAT(latitude, latitude)
CSB_FIELD_SETTER_FLOAT(longitude, longitude)
CSB_FIELD_SETTER_FIXED(rotation_period, rotationPeriod)
CSB_FIELD_SETTER_FIXED(axial_tilt, axialTilt)
CSB_FIELD_SETTER_FIXED(metallicity, metallicity)
CSB_FIELD_SETTER_FIXED(volcanicity, volcanicity)
CSB_FIELD_SETTER_FIXED(atmos_density, volatileGas)
CSB_FIELD_SETTER_FIXED(atmos_oxidizing, atmosOxidizing)
CSB_FIELD_SETTER_FIXED(ocean_cover, volatileLiquid)
CSB_FIELD_SETTER_FIXED(ice_cover, volatileIces)
CSB_FIELD_SETTER_FIXED(life, life)

#undef CSB_FIELD_SETTER_FIXED
#undef CSB_FIELD_SETTER_FLOAT
#undef CSB_FIELD_SETTER_INT

static int l_csb_seed(lua_State *L)
{
	CustomSystemBody *csb = l_csb_check(L, 1);
	csb->seed = luaL_checkinteger(L, 2);
	csb->want_rand_seed = false;
	lua_settop(L, 1);
	return 1;
}

static int l_csb_orbital_offset(lua_State *L)
{
	CustomSystemBody *csb = l_csb_check(L, 1);
	const fixed *value = LuaFixed::CheckFromLua(L, 2);
	csb->orbitalOffset = *value;
	csb->want_rand_offset = false;
	lua_settop(L, 1);
	return 1;
}

static int l_csb_height_map(lua_State *L)
{
	CustomSystemBody *csb = l_csb_check(L, 1);
	const char *fname = luaL_checkstring(L, 2);
	int fractal = luaL_checkinteger(L, 3);
	if (fractal >= 2) { return luaL_error(L, "invalid terrain fractal type"); }

	csb->heightMapFilename = FileSystem::JoinPathBelow("heightmaps", fname);
	csb->heightMapFractal = fractal;
	lua_settop(L, 1);
	return 1;
}

static int l_csb_rings(lua_State *L)
{
	CustomSystemBody *csb = l_csb_check(L, 1);
	if (lua_isboolean(L, 2)) {
		if (lua_toboolean(L, 2)) {
			csb->ringStatus = CustomSystemBody::WANT_RINGS;
		} else {
			csb->ringStatus = CustomSystemBody::WANT_NO_RINGS;
		}
	} else {
		csb->ringStatus = CustomSystemBody::WANT_CUSTOM_RINGS;
		csb->ringInnerRadius = *LuaFixed::CheckFromLua(L, 2);
		csb->ringOuterRadius = *LuaFixed::CheckFromLua(L, 3);
		luaL_checktype(L, 4, LUA_TTABLE);
		Color4f col;
		lua_rawgeti(L, 4, 1);
		col.r = luaL_checknumber(L, -1);
		lua_rawgeti(L, 4, 2);
		col.g = luaL_checknumber(L, -1);
		lua_rawgeti(L, 4, 3);
		col.b = luaL_checknumber(L, -1);
		lua_rawgeti(L, 4, 4);
		col.a = luaL_optnumber(L, -1, 0.85); // default alpha value
		csb->ringColor = col;
	}
	lua_settop(L, 1);
	return 1;
}

static int l_csb_gc(lua_State *L)
{
	CustomSystemBody **csbptr = static_cast<CustomSystemBody**>(
			luaL_checkudata(L, 1, LuaCustomSystemBody_TypeName));
	delete *csbptr; // does nothing if *csbptr is null
	*csbptr = 0;
	return 0;
}

static luaL_Reg LuaCustomSystemBody_meta[] = {
	{ "new", &l_csb_new },
	{ "seed", &l_csb_seed },
	{ "radius", &l_csb_radius },
	{ "mass", &l_csb_mass },
	{ "temp", &l_csb_temp },
	{ "semi_major_axis", &l_csb_semi_major_axis },
	{ "eccentricity", &l_csb_eccentricity },
	{ "orbital_offset", &l_csb_orbital_offset },
	{ "latitude", &l_csb_latitude },
	// latitude is for surface bodies, inclination is for orbiting bodies (but they're the same field)
	{ "inclination", &l_csb_latitude },
	{ "longitude", &l_csb_longitude },
	{ "rotation_period", &l_csb_rotation_period },
	{ "axial_tilt", &l_csb_axial_tilt },
	{ "height_map", &l_csb_height_map },
	{ "metallicity", &l_csb_metallicity },
	{ "volcanicity", &l_csb_volcanicity },
	{ "atmos_density", &l_csb_atmos_density },
	{ "atmos_oxidizing", &l_csb_atmos_oxidizing },
	{ "ocean_cover", &l_csb_ocean_cover },
	{ "ice_cover", &l_csb_ice_cover },
	{ "life", &l_csb_life },
	{ "rings", &l_csb_rings },
	{ "__gc", &l_csb_gc },
	{ 0, 0 }
};

// ------- CustomSystem --------

static const char LuaCustomSystem_TypeName[] = "CustomSystem";

static CustomSystem **l_csys_check_ptr(lua_State *L, int idx) {
	CustomSystem **csptr = static_cast<CustomSystem**>(
			luaL_checkudata(L, idx, LuaCustomSystem_TypeName));
	if (!(*csptr)) {
		abort();
		luaL_error(L, "invalid system (this system has already been used)");
	}
	return csptr;
}

static CustomSystem *l_csys_check(lua_State *L, int idx)
{ return *l_csys_check_ptr(L, idx); }

static int interpret_star_types(int *starTypes, lua_State *L, int idx)
{
	LUA_DEBUG_START(L);
	luaL_checktype(L, idx, LUA_TTABLE);
	lua_pushvalue(L, idx);
	int i;
	for (i = 0; i < 4; ++i) {
		int ty = SystemBody::TYPE_GRAVPOINT;
		lua_rawgeti(L, -1, i + 1);
		if (lua_type(L, -1) == LUA_TSTRING) {
			ty = LuaConstants::GetConstantFromArg(L, "BodyType", -1);
			if (ty < SystemBody::TYPE_STAR_MIN || ty > SystemBody::TYPE_STAR_MAX) {
				luaL_error(L, "system star %d does not have a valid star type", i+1);
				// unreachable (longjmp in luaL_error)
			}
		} else if (!lua_isnil(L, -1)) {
			luaL_error(L, "system star %d is not a string constant", i+1);
		}
		lua_pop(L, 1);
		LUA_DEBUG_CHECK(L, 1);

		starTypes[i] = ty;
		if (ty == SystemBody::TYPE_GRAVPOINT) break;
	}
	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
	return i;
}

static int l_csys_new(lua_State *L)
{
	const char *name = luaL_checkstring(L, 2);
	int starTypes[4];
	int numStars = interpret_star_types(starTypes, L, 3);

	CustomSystem **csptr = static_cast<CustomSystem**>(
			lua_newuserdata(L, sizeof(CustomSystem*)));
	*csptr = new CustomSystem;
	luaL_setmetatable(L, LuaCustomSystem_TypeName);

	(*csptr)->name = name;
	(*csptr)->numStars = numStars;
	assert(numStars <= 4);
	for (int i = 0; i < numStars; ++i)
		(*csptr)->primaryType[i] = static_cast<SystemBody::BodyType>(starTypes[i]);
	return 1;
}

static int l_csys_seed(lua_State *L)
{
	CustomSystem *cs = l_csys_check(L, 1);
	cs->seed = luaL_checkinteger(L, 2);
	lua_settop(L, 1);
	return 1;
}

static int l_csys_explored(lua_State *L)
{
	CustomSystem *cs = l_csys_check(L, 1);
	cs->explored = lua_toboolean(L, 2);
	cs->want_rand_explored = false;
	lua_settop(L, 1);
	return 1;
}

static int l_csys_short_desc(lua_State *L)
{
	CustomSystem *cs = l_csys_check(L, 1);
	cs->shortDesc = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

static int l_csys_long_desc(lua_State *L)
{
	CustomSystem *cs = l_csys_check(L, 1);
	cs->longDesc = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

static int l_csys_govtype(lua_State *L)
{
	CustomSystem *cs = l_csys_check(L, 1);
	cs->govType = static_cast<Polit::GovType>(LuaConstants::GetConstantFromArg(L, "PolitGovType", 2));
	lua_settop(L, 1);
	return 1;
}

static void _add_children_to_sbody(lua_State *L, CustomSystemBody *sbody)
{
	lua_checkstack(L, 5); // grow the stack if necessary
	LUA_DEBUG_START(L);
	int i = 0;
	while (true) {
		// first there's a body
		lua_rawgeti(L, -1, ++i);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}
		LUA_DEBUG_CHECK(L, 1);
		CustomSystemBody **csptr = l_csb_check_ptr(L, -1);
		CustomSystemBody *kid = *csptr;
		*csptr = 0;
		lua_pop(L, 1);
		LUA_DEBUG_CHECK(L, 0);

		// then there are any number of sub-tables containing direct children
		while (true) {
			lua_rawgeti(L, -1, i+1);
			LUA_DEBUG_CHECK(L, 1);
			if (!lua_istable(L, -1)) break;
			_add_children_to_sbody(L, kid);
			lua_pop(L, 1);
			LUA_DEBUG_CHECK(L, 0);
			++i;
		}
		lua_pop(L, 1);
		LUA_DEBUG_CHECK(L, 0);

		//printf("add-children-to-body adding %s to %s\n", kid->name.c_str(), sbody->name.c_str());

		sbody->children.push_back(kid);
	}
	//printf("add-children-to-body done for %s\n", sbody->name.c_str());
	LUA_DEBUG_END(L, 0);
}

static int l_csys_bodies(lua_State *L)
{
	CustomSystem *cs = l_csys_check(L, 1);
	CustomSystemBody **primary_ptr = l_csb_check_ptr(L, 2);
	int primary_type = (*primary_ptr)->type;
	luaL_checktype(L, 3, LUA_TTABLE);

	if (primary_type < SystemBody::TYPE_STAR_MIN || primary_type > SystemBody::TYPE_STAR_MAX)
		return luaL_error(L, "first body does not have a valid star type");
	if (primary_type != cs->primaryType[0])
		return luaL_error(L, "first body type does not match the system's primary star type");

	lua_pushvalue(L, 3);
	_add_children_to_sbody(L, *primary_ptr);
	lua_pop(L, 1);

	cs->sBody = *primary_ptr;
	*primary_ptr = 0;

	lua_settop(L, 1);
	return 1;
}

static int l_csys_add_to_sector(lua_State *L)
{
	CustomSystem **csptr = l_csys_check_ptr(L, 1);

	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int z = luaL_checkinteger(L, 4);
	const vector3d *v = LuaVector::CheckFromLua(L, 5);

	(*csptr)->sectorX = x;
	(*csptr)->sectorY = y;
	(*csptr)->sectorZ = z;
	(*csptr)->pos = vector3f(*v);

	//printf("l_csys_add_to_sector: %s added to %d, %d, %d\n", (*csptr)->name.c_str(), x, y, z);

	s_sectorMap[SystemPath(x, y, z)].push_back(*csptr);
	*csptr = 0;
	return 0;
}

static int l_csys_gc(lua_State *L)
{
	CustomSystem **csptr = static_cast<CustomSystem**>(
			luaL_checkudata(L, 1, LuaCustomSystem_TypeName));
	delete *csptr;
	*csptr = 0;
	return 0;
}

static luaL_Reg LuaCustomSystem_meta[] = {
	{ "new", &l_csys_new },
	{ "seed", &l_csys_seed },
	{ "explored", &l_csys_explored },
	{ "short_desc", &l_csys_short_desc },
	{ "long_desc", &l_csys_long_desc },
	{ "govtype", &l_csys_govtype },
	{ "bodies", &l_csys_bodies },
	{ "add_to_sector", &l_csys_add_to_sector },
	{ "__gc", &l_csys_gc },
	{ 0, 0 }
};

// ------ CustomSystem initialisation ------

static void register_class(lua_State *L, const char *tname, luaL_Reg *meta)
{
	LUA_DEBUG_START(L);
	luaL_newmetatable(L, tname);
	luaL_setfuncs(L, meta, 0);

	// map the metatable to its own __index
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// publish the metatable
	lua_setglobal(L, tname);

	LUA_DEBUG_END(L, 0);
}

static void RegisterCustomSystemsAPI(lua_State *L)
{
	register_class(L, LuaCustomSystem_TypeName, LuaCustomSystem_meta);
	register_class(L, LuaCustomSystemBody_TypeName, LuaCustomSystemBody_meta);
}

void CustomSystem::Init()
{
	lua_State *L = luaL_newstate();
	LUA_DEBUG_START(L);

	pi_lua_open_standard_base(L);

	LuaVector::Register(L);
	LuaFixed::Register(L);
	LuaConstants::Register(L);

	// create a shortcut f = fixed.new
	lua_getglobal(L, LuaFixed::LibName);
	lua_getfield(L, -1, "new");
	assert(lua_iscfunction(L, -1));
	lua_setglobal(L, "f");
	lua_pop(L, 1); // pop the fixed table

	// provide shortcut vector constructor: v = vector.new
	lua_getglobal(L, LuaVector::LibName);
	lua_getfield(L, -1, "new");
	assert(lua_iscfunction(L, -1));
	lua_setglobal(L, "v");
	lua_pop(L, 1); // pop the vector table

	LUA_DEBUG_CHECK(L, 0);

	RegisterCustomSystemsAPI(L);

	LUA_DEBUG_CHECK(L, 0);
	pi_lua_dofile_recursive(L, "systems");

	LUA_DEBUG_END(L, 0);
	lua_close(L);
}

void CustomSystem::Uninit()
{
	for (SectorMap::iterator secIt = s_sectorMap.begin(); secIt != s_sectorMap.end(); ++secIt) {
		for (CustomSystem::SystemList::iterator
				sysIt = secIt->second.begin(); sysIt != secIt->second.end(); ++sysIt) {
			delete *sysIt;
		}
	}
	s_sectorMap.clear();
}

const CustomSystem::SystemList &CustomSystem::GetCustomSystemsForSector(int x, int y, int z)
{
	SystemPath path(x,y,z);
	SectorMap::const_iterator it = s_sectorMap.find(path);
	return (it != s_sectorMap.end()) ? it->second : s_emptySystemList;
}

CustomSystem::CustomSystem():
	sBody(0),
	numStars(0),
	seed(0),
	want_rand_explored(true),
	govType(Polit::GOV_NONE)
{
	for (int i = 0; i < 4; ++i)
		primaryType[i] = SystemBody::TYPE_GRAVPOINT;
}

CustomSystem::~CustomSystem()
{
	delete sBody;
}

CustomSystemBody::CustomSystemBody():
	averageTemp(0),
	want_rand_offset(true),
	latitude(0.0),
	longitude(0.0),
	ringStatus(WANT_RANDOM_RINGS),
	seed(0),
	want_rand_seed(true)
{}

CustomSystemBody::~CustomSystemBody()
{
	for (std::vector<CustomSystemBody*>::iterator
			it = children.begin(); it != children.end(); ++it) {
		delete (*it);
	}
}
