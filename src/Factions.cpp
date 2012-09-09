#include "Factions.h"
#include "galaxy/SystemPath.h"

#include "LuaUtils.h"
#include "LuaVector.h"
#include "LuaFixed.h"
#include "LuaConstants.h"
#include "Polit.h"
#include "FileSystem.h"

typedef std::vector<Faction*>	TFactions;
typedef TFactions::iterator		TFactionsIterator;
TFactions						s_factions;

// ------- Faction --------

static const char LuaFaction_TypeName[] = "Faction";

static Faction **l_fac_check_ptr(lua_State *L, int idx) {
	Faction **csbptr = static_cast<Faction**>(
			luaL_checkudata(L, idx, LuaFaction_TypeName));
	if (!(*csbptr)) {
		abort();
		luaL_argerror(L, idx, "invalid body (this body has already been used)");
	}
	return csbptr;
}

static Faction *l_fac_check(lua_State *L, int idx)
{ return *l_fac_check_ptr(L, idx); }

static int l_fac_new(lua_State *L)
{
	const char *pName = luaL_checkstring(L, 2);
	
	Faction **csbptr = static_cast<Faction**>(lua_newuserdata(L, sizeof(Faction*)));
	*csbptr = new Faction;
	luaL_setmetatable(L, LuaFaction_TypeName);

	(*csbptr)->name = pName;

	return 1;
}

#define LFAC_FIELD_SETTER_FIXED(luaname, fieldname)         \
	static int l_fac_ ## luaname (lua_State *L) {          \
		Faction *csb = l_fac_check(L, 1);				   \
		const fixed *value = LuaFixed::CheckFromLua(L, 2); \
		csb->fieldname = *value;                           \
		lua_settop(L, 1); return 1;                        \
	}

#define LFAC_FIELD_SETTER_FLOAT(luaname, fieldname)         \
	static int l_fac_ ## luaname (lua_State *L) {          \
		Faction *csb = l_fac_check(L, 1);				   \
		double value = luaL_checknumber(L, 2);             \
		csb->fieldname = value;                            \
		lua_settop(L, 1); return 1;                        \
	}

#define LFAC_FIELD_SETTER_INT(luaname, fieldname)           \
	static int l_fac_ ## luaname (lua_State *L) {          \
		Faction *csb = l_fac_check(L, 1);				   \
		int value = luaL_checkinteger(L, 2);               \
		csb->fieldname = value;                            \
		lua_settop(L, 1); return 1;                        \
	}

static int l_fac_description_short(lua_State *L)
{
	Faction *csb = l_fac_check(L, 1);
	csb->description_short = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

static int l_fac_description(lua_State *L)
{
	Faction *csb = l_fac_check(L, 1);
	csb->description = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

static int l_fac_govtype(lua_State *L)
{
	Faction *csb = l_fac_check(L, 1);
	csb->govType = static_cast<Polit::GovType>(LuaConstants::GetConstantFromArg(L, "PolitGovType", 2));
	lua_settop(L, 1);
	return 1;
}

// sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
static int l_fac_homeworld (lua_State *L) 
{
	Faction *csb = l_fac_check(L, 1);
	int value1 = luaL_checkinteger(L, 2);
	int value2 = luaL_checkinteger(L, 3);
	int value3 = luaL_checkinteger(L, 4);
	int value4 = luaL_checkinteger(L, 5);
	int value5 = luaL_checkinteger(L, 6);
	csb->homeworld = SystemPath(value1,value2,value3,value4,value5);
	csb->hasHomeworld = true;
	lua_settop(L, 1); 
	return 1;
}

LFAC_FIELD_SETTER_FLOAT(foundingDate,foundingDate)		// date faction came into existence
LFAC_FIELD_SETTER_FLOAT(expansionRate,expansionRate)		// lightyears per year that the volume expands.

static int l_fac_military_name(lua_State *L)
{
	Faction *csb = l_fac_check(L, 1);
	csb->military_name = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}
//military logo
static int l_fac_police_name(lua_State *L)
{
	Faction *csb = l_fac_check(L, 1);
	csb->police_name = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}
//police logo
//goods/equipment availability (1-per-economy-type: aka agricultural, industrial, tourist, etc)
//goods/equipment legality
static int l_fac_illegal_goods_probability(lua_State *L)
{
	Faction *csb = l_fac_check(L, 1);
	const char *typeName = luaL_checkstring(L, 2);
	const Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(L, "EquipType", typeName));
	const int probability = luaL_checkinteger(L, 3);
	const bool equality_test = luaL_checkinteger(L, 4)==0;

	if (e < 0 || e >= Equip::MISSILE_UNGUIDED) {
		pi_lua_warn(L,
			"argument out of range: Faction{%s}:IllegalGoodsProbability('%s', %d, %d)",
			csb->name, typeName, probability, equality_test);
		return 0;
	}

	csb->equip_legality[e] = Faction::ProbEqualityPair(probability,equality_test);
	lua_settop(L, 1); 

	return 1;
}
//ship availability

static int l_fac_colour(lua_State *L)
{
	Faction *csb = l_fac_check(L, 1);
	const float r = luaL_checknumber(L, 2);
	const float g = luaL_checknumber(L, 3);
	const float b = luaL_checknumber(L, 4);

	csb->colour = Color(r,g,b);

	lua_settop(L, 1); 

	return 1;
}

#undef LFAC_FIELD_SETTER_FIXED
#undef LFAC_FIELD_SETTER_FLOAT
#undef LFAC_FIELD_SETTER_INT

static int l_csys_add_to_factions(lua_State *L)
{
	Faction **csptr = l_fac_check_ptr(L, 1);

	const std::string FactionName = luaL_checkstring(L, 2);

	printf("l_csys_add_to_factions: %s added under name: %s\n", (*csptr)->name.c_str(), FactionName.c_str());

	s_factions.push_back(*csptr);
	*csptr = 0;
	return 0;
}

static int l_fac_gc(lua_State *L)
{
	Faction **csbptr = static_cast<Faction**>(
			luaL_checkudata(L, 1, LuaFaction_TypeName));
	delete *csbptr; // does nothing if *csbptr is null
	*csbptr = 0;
	return 0;
}

static luaL_Reg LuaFaction_meta[] = {
	{ "new", &l_fac_new },
	{ "description_short", &l_fac_description_short },
	{ "description", &l_fac_description },
	{ "govtype", &l_fac_govtype },
	{ "homeworld", &l_fac_homeworld },
	{ "foundingDate", &l_fac_foundingDate },
	{ "expansionRate", &l_fac_expansionRate },
	{ "military_name", &l_fac_military_name },
	{ "police_name", &l_fac_police_name },
	{ "illegal_goods_probability", &l_fac_illegal_goods_probability },
	{ "colour", &l_fac_colour },
	{ "add_to_factions", &l_csys_add_to_factions },
	{ "__gc", &l_fac_gc },
	{ 0, 0 }
};

// ------ Factions initialisation ------

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

static void RegisterFactionsAPI(lua_State *L)
{
	register_class(L, LuaFaction_TypeName, LuaFaction_meta);
}

//static 
void Faction::Init()
{
	lua_State *L = luaL_newstate();
	LUA_DEBUG_START(L);

	luaL_requiref(L, "_G", &luaopen_base, 1);
	luaL_requiref(L, LUA_DBLIBNAME, &luaopen_debug, 1);
	luaL_requiref(L, LUA_MATHLIBNAME, &luaopen_math, 1);
	lua_pop(L, 3);

	LuaVector::Register(L);
	LuaFixed::Register(L);
	LuaConstants::Register(L);

	// create an alias math.deg2rad = math.rad
	lua_getglobal(L, LUA_MATHLIBNAME);
	lua_getfield(L, -1, "rad");
	assert(lua_isfunction(L, -1));
	lua_setfield(L, -2, "deg2rad");
	lua_pop(L, 1); // pop the math table

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

	RegisterFactionsAPI(L);

	LUA_DEBUG_CHECK(L, 0);
	pi_lua_dofile_recursive(L, "factions");

	LUA_DEBUG_END(L, 0);
	lua_close(L);

	printf("Number of factions added: %u\n", s_factions.size());
}

//static 
void Faction::Uninit()
{
	for (TFactionsIterator facIter = s_factions.begin(); facIter != s_factions.end(); ++facIter) {
		delete (*facIter);
	}
	s_factions.clear();
}

//static 
const Faction *Faction::GetFaction(const std::string &nameIdx)
{
	for (TFactionsIterator facIter = s_factions.begin(); facIter != s_factions.end(); ++facIter) {
		if( (*facIter)->name == nameIdx ) {
			delete (*facIter);
		}
	}
	return NULL;
}

//static 
const Faction *Faction::GetFaction(const Uint32 index)
{
	assert( index<s_factions.size() );
	return s_factions[index];
}

//static 
const Uint32 Faction::GetNumFactions()
{
	return s_factions.size();
}

//static 
const Uint32 Faction::GetNearestFactionIndex(const SystemPath& sysPath)
{
	// Iterate through all of the factions and find the one nearest to the system we're checking it against.
	const vector3f sysPos(sysPath.sectorX, sysPath.sectorY, sysPath.sectorZ);
	const Faction *pFoundFaction = nullptr;
	Sint32 nearestDistance = INT_MAX;

	// get the current year
	// NB: cannot access the PI::game->GetTime() method here as game is NULL when deserialised from save game -
	//	- I had hoped to use this to give a simple expanding spherical volume to each faction. Use 3200 as the-
	//	- base year, all factions should have come into existence prior to this date.
	const double current_year = 3200;//get_year(Pi::game->GetTime());

	// iterate
	Uint32 ret_index = 0;
	for (Uint32 index = 0;  index<s_factions.size(); ++index) {
		const Faction *ptr = s_factions[index];
		assert(ptr);
		if( !ptr->hasHomeworld && nullptr==pFoundFaction )
		{
			// We've not yet found a faction that we're within the radius of
			// and we're currently iterating over a faction that is decentralised (probably Independent)
			pFoundFaction = ptr;
			ret_index = index;
		}
		else if( ptr->hasHomeworld )
		{
			// We can end early here if they're the same as factions homeworld like Earth or Achernar
			// or should this be ptr->homeworld.IsSameSystem(sysPath) ???
			if( ptr->homeworld.IsSameSector(sysPath) ) { 
				pFoundFaction = ptr;
				return index;
			}

			// get the distance
			const vector3f hm(ptr->homeworld.sectorX, ptr->homeworld.sectorY, ptr->homeworld.sectorZ);
			const Sint32 distance = (sysPos - hm).Length();

			// 
			double radius = (current_year - ptr->foundingDate) * ptr->expansionRate;

			// check we've found a closer faction
			if( distance <= radius && distance < nearestDistance ) {
				nearestDistance = distance;
				pFoundFaction = ptr;
				ret_index = index;
			}
		}
	}

	return ret_index;
}

Faction::Faction() :
	govType(Polit::GOV_INVALID), 
	hasHomeworld(false), 
	foundingDate(0.0),
	expansionRate(0.0)
{
}

Faction::~Faction()
{
	Uninit();
}
