// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Factions.h"
#include "galaxy/SystemPath.h"
#include "galaxy/Sector.h"

#include "LuaUtils.h"
#include "LuaVector.h"
#include "LuaFixed.h"
#include "LuaConstants.h"
#include "Polit.h"
#include "FileSystem.h"

const Uint32 Faction::BAD_FACTION_IDX = UINT_MAX;

typedef std::vector<Faction>  FactionList;
typedef FactionList::iterator FactionIterator;
static FactionList            s_factions;

// ------- Faction --------

static const char LuaFaction_TypeName[] = "Faction";

static Faction **l_fac_check_ptr(lua_State *L, int idx) {
	Faction **facptr = static_cast<Faction**>(
		luaL_checkudata(L, idx, LuaFaction_TypeName));
	if (!(*facptr)) {
		luaL_argerror(L, idx, "invalid body (this body has already been used)");
		abort();
	}
	return facptr;
}

static Faction *l_fac_check(lua_State *L, int idx)
{
	return *l_fac_check_ptr(L, idx);
}

static int l_fac_new(lua_State *L)
{
	const char *name = luaL_checkstring(L, 2);

	Faction **facptr = static_cast<Faction**>(lua_newuserdata(L, sizeof(Faction*)));
	*facptr = new Faction;
	luaL_setmetatable(L, LuaFaction_TypeName);

	(*facptr)->name = name;

	return 1;
}

#define LFAC_FIELD_SETTER_FIXED(luaname, fieldname)        \
	static int l_fac_ ## luaname (lua_State *L) {          \
		Faction *fac = l_fac_check(L, 1);                  \
		const fixed *value = LuaFixed::CheckFromLua(L, 2); \
		fac->fieldname = *value;                           \
		lua_settop(L, 1); return 1;                        \
	}

#define LFAC_FIELD_SETTER_FLOAT(luaname, fieldname)        \
	static int l_fac_ ## luaname (lua_State *L) {          \
		Faction *fac = l_fac_check(L, 1);                  \
		double value = luaL_checknumber(L, 2);             \
		fac->fieldname = value;                            \
		lua_settop(L, 1); return 1;                        \
	}

#define LFAC_FIELD_SETTER_INT(luaname, fieldname)          \
	static int l_fac_ ## luaname (lua_State *L) {          \
		Faction *fac = l_fac_check(L, 1);                  \
		int value = luaL_checkinteger(L, 2);               \
		fac->fieldname = value;                            \
		lua_settop(L, 1); return 1;                        \
	}

static int l_fac_description_short(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	fac->description_short = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

static int l_fac_description(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	fac->description = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

static int l_fac_govtype(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	fac->govType = static_cast<Polit::GovType>(LuaConstants::GetConstantFromArg(L, "PolitGovType", 2));
	lua_settop(L, 1);
	return 1;
}

// sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
static int l_fac_homeworld (lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	Sint32 x = luaL_checkinteger(L, 2);
	Sint32 y = luaL_checkinteger(L, 3);
	Sint32 z = luaL_checkinteger(L, 4);
	Uint32 si = luaL_checkinteger(L, 5);
	Uint32 bi = luaL_checkinteger(L, 6);
	fac->homeworld = SystemPath(x,y,z,si,bi);
	fac->hasHomeworld = true;
	lua_settop(L, 1);
	return 1;
}

LFAC_FIELD_SETTER_FLOAT(foundingDate, foundingDate)   // date faction came into existence
LFAC_FIELD_SETTER_FLOAT(expansionRate, expansionRate) // lightyears per year that the volume expands.

static int l_fac_military_name(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	fac->military_name = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

//military logo
static int l_fac_police_name(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	fac->police_name = luaL_checkstring(L, 2);
	lua_settop(L, 1);
	return 1;
}

//police logo
//goods/equipment availability (1-per-economy-type: aka agricultural, industrial, tourist, etc)
//goods/equipment legality
static int l_fac_illegal_goods_probability(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	const char *typeName = luaL_checkstring(L, 2);
	const Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(L, "EquipType", typeName));
	const uint32_t probability = luaL_checkunsigned(L, 3);

	if (e < Equip::FIRST_COMMODITY || e > Equip::LAST_COMMODITY) {
		pi_lua_warn(L,
			"argument out of range: Faction{%s}:IllegalGoodsProbability('%s', %d)",
			fac->name.c_str(), typeName, probability);
		return 0;
	}

	if (probability > 100) {
		pi_lua_warn(L,
			"argument (probability 0-100) out of range: Faction{%s}:IllegalGoodsProbability('%s', %d)",
			fac->name.c_str(), typeName, probability);
		return 0;
	}

	fac->equip_legality[e] = probability;
	lua_settop(L, 1);

	return 1;
}

//ship availability
static int l_fac_colour(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	const float r = luaL_checknumber(L, 2);
	const float g = luaL_checknumber(L, 3);
	const float b = luaL_checknumber(L, 4);

	fac->colour = Color(r,g,b);

	lua_settop(L, 1);

	return 1;
}

#undef LFAC_FIELD_SETTER_FIXED
#undef LFAC_FIELD_SETTER_FLOAT
#undef LFAC_FIELD_SETTER_INT

static int l_fac_add_to_factions(lua_State *L)
{
	Faction **facptr = l_fac_check_ptr(L, 1);

	const std::string factionName(luaL_checkstring(L, 2));

	printf("l_fac_add_to_factions: added '%s' [%s]\n", (*facptr)->name.c_str(), factionName.c_str());

	s_factions.push_back(**facptr);

	return 0;
}

static int l_fac_gc(lua_State *L)
{
	Faction **facptr = static_cast<Faction**>(
			luaL_checkudata(L, 1, LuaFaction_TypeName));
	delete *facptr; // does nothing if *facptr is null
	return 0;
}

static luaL_Reg LuaFaction_meta[] = {
	{ "new",                       &l_fac_new },
	{ "description_short",         &l_fac_description_short },
	{ "description",               &l_fac_description },
	{ "govtype",                   &l_fac_govtype },
	{ "homeworld",                 &l_fac_homeworld },
	{ "foundingDate",              &l_fac_foundingDate },
	{ "expansionRate",             &l_fac_expansionRate },
	{ "military_name",             &l_fac_military_name },
	{ "police_name",               &l_fac_police_name },
	{ "illegal_goods_probability", &l_fac_illegal_goods_probability },
	{ "colour",                    &l_fac_colour },
	{ "add_to_factions",           &l_fac_add_to_factions },
	{ "__gc",                      &l_fac_gc },
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

	pi_lua_open_standard_base(L);

	LuaConstants::Register(L);

	LUA_DEBUG_CHECK(L, 0);

	RegisterFactionsAPI(L);

	LUA_DEBUG_CHECK(L, 0);
	pi_lua_dofile_recursive(L, "factions");

	LUA_DEBUG_END(L, 0);
	lua_close(L);

	printf("Number of factions added: " SIZET_FMT "\n", s_factions.size());
}

void Faction::Uninit()
{
	s_factions.clear();
}

const Faction *Faction::GetFaction(const Uint32 index)
{
	assert( index < s_factions.size() );
	return &s_factions[index];
}

const Uint32 Faction::GetNumFactions()
{
	return s_factions.size();
}

const Uint32 Faction::GetNearestFactionIndex(const SystemPath& sysPath)
{
	// firstly is this a custom StarSystem which might have funny settings
	Sector sec(sysPath.sectorX, sysPath.sectorY, sysPath.sectorZ);
	Polit::GovType a = Polit::GOV_INVALID;

	/* from custom system definition */
	if (sec.m_systems[sysPath.systemIndex].customSys) {
		Polit::GovType t = sec.m_systems[sysPath.systemIndex].customSys->govType;
		a = t;
	}
	// if the custom system has a valid govType set then try to find a matching faction
	if( a != Polit::GOV_INVALID )
	{
		for (Uint32 index = 0; index < s_factions.size(); ++index) {
			const Faction &fac = s_factions[index];
			if(fac.govType == a) {
				return index;
			}
		}
		// no matching faction found, return the default
		return BAD_FACTION_IDX;
	}
	// if we don't find a match then we can go on and assign it a faction allegiance like normal below...

	// Iterate through all of the factions and find the one nearest to the system we're checking it against.
	const Faction *foundFaction = 0;
	Sint32 nearestDistance = INT_MAX;

	// get the current year
	// XXX: cannot access the PI::game->GetTime() method here as game is NULL when deserialised from save game -
	//	- I had hoped to use this to give a simple expanding spherical volume to each faction. Use 3200 as the-
	//	- base year, all factions should have come into existence prior to this date.
	const double current_year = 3200;//get_year(Pi::game->GetTime());

	// iterate
	Uint32 ret_index = BAD_FACTION_IDX;
	for (Uint32 index = 0; index < s_factions.size(); ++index) {
		const Faction &fac = s_factions[index];

		if( !fac.hasHomeworld && !foundFaction ) {
			// We've not yet found a faction that we're within the radius of
			// and we're currently iterating over a faction that is decentralised (probably Independent)
			foundFaction = &fac;
			ret_index = index;
		}
		else if( fac.hasHomeworld ) {
			// We can end early here if they're the same as factions homeworld like Earth or Achernar
			if( fac.homeworld.IsSameSector(sysPath) ) {
				foundFaction = &fac;
				return index;
			}

			// get the distance
			const Sector sec1(fac.homeworld.sectorX, fac.homeworld.sectorY, fac.homeworld.sectorZ);
			const Sector sec2(sysPath.sectorX, sysPath.sectorY, sysPath.sectorZ);
			const double distance = Sector::DistanceBetween(&sec1, fac.homeworld.systemIndex, &sec2, sysPath.systemIndex);

			// calculate the current radius the faction occupies
			const double radius = (current_year - fac.foundingDate) * fac.expansionRate;

			// check we've found a closer faction
			if( (distance <= radius) && (distance < nearestDistance) ) {
				nearestDistance = distance;
				foundFaction = &fac;
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
}
