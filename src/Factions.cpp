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

typedef std::vector<Faction*>  FactionList;
typedef FactionList::iterator FactionIterator;
typedef std::map<std::string, Uint32> FactionIndexes;

static FactionList    s_factions;
static FactionIndexes s_factions_indexes;

// ------- Faction --------

struct FactionBuilder {
	Faction *fac;
	bool registered;
	bool skip;
};

static const char LuaFaction_TypeName[] = "Faction";

static FactionBuilder *l_fac_check_builder(lua_State *L, int idx) {
	FactionBuilder *facbld = static_cast<FactionBuilder*>(
		luaL_checkudata(L, idx, LuaFaction_TypeName));
	assert(facbld->fac);
	return facbld;
}

static Faction *l_fac_check(lua_State *L, int idx)
{
	return l_fac_check_builder(L, idx)->fac;
}

static int l_fac_new(lua_State *L)
{
	const char *name = luaL_checkstring(L, 2);

	FactionBuilder *facbld = static_cast<FactionBuilder*>(lua_newuserdata(L, sizeof(*facbld)));
	facbld->fac = new Faction;
	facbld->registered = false;
	facbld->skip       = false;
	luaL_setmetatable(L, LuaFaction_TypeName);

	facbld->fac->name = name;

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

// weightings to use when picking a government type
static int l_fac_govtype_weight(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	const char *typeName = luaL_checkstring(L, 2);
	const Polit::GovType g = static_cast<Polit::GovType>(LuaConstants::GetConstant(L, "PolitGovType", typeName));
	const int32_t weight = luaL_checkint(L, 3);	// signed as we will need to compare with signed out of MTRand.Int32

	if (g < Polit::GOV_RAND_MIN || g > Polit::GOV_RAND_MAX) {
		pi_lua_warn(L,
			"government type out of range: Faction{%s}:govtype_weight('%s', %d)",
			fac->name.c_str(), typeName, weight);
		return 0;
	}

	if (weight < 0) {
		pi_lua_warn(L,
			"weight must a postive integer: Faction{%s}:govtype_weight('%s', %d)",
			fac->name.c_str(), typeName, weight);
		return 0;		
	}

	fac->govtype_weights.push_back(std::make_pair(g, weight));
	fac->govtype_weights_total += weight;
	lua_settop(L, 1);

	return 1;
}

// sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
static int l_fac_homeworld (lua_State *L)
{
	FactionBuilder *facbld = l_fac_check_builder(L, 1);
	Faction *fac = facbld->fac;
	Sint32 x = luaL_checkinteger(L, 2);
	Sint32 y = luaL_checkinteger(L, 3);
	Sint32 z = luaL_checkinteger(L, 4);
	Sint32 si = luaL_checkinteger(L, 5);
	Uint32 bi = luaL_checkinteger(L, 6);

	fac->SetBestFitHomeworld(x, y, z, si, bi);
	fac->hasHomeworld = true;
	facbld->skip      = !fac->homeworld.HasValidSystem();	// wasn't a valid system
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
	FactionBuilder *facbld = l_fac_check_builder(L, 1);
	Faction *fac = facbld->fac;

	const std::string factionName(luaL_checkstring(L, 2));

	if (!facbld->registered && !facbld->skip) {
		if (facbld->fac->hasHomeworld) {
			printf("l_fac_add_to_factions: added (%3d,%3d,%3d) f=%4.0f e=%2.2f '%s' [%s]\n"
				, fac->homeworld.sectorX, fac->homeworld.sectorY, fac->homeworld.sectorZ, fac->foundingDate, fac->expansionRate, fac->name.c_str(), factionName.c_str());
		}
		else {
			printf("l_fac_add_to_factions: added '%s' [%s]\n", fac->name.c_str(), factionName.c_str());
		}

		s_factions.push_back(facbld->fac);
		s_factions_indexes[facbld->fac->name] = s_factions.size()-1;
		facbld->registered = true;
		return 0;
	} else if (facbld->skip) {
		printf("l_fac_add_to_factions: invalid homeworld, skipped (%3d,%3d,%3d) f=%4.0f e=%2.2f '%s' [%s]\n"
				, fac->homeworld.sectorX, fac->homeworld.sectorY, fac->homeworld.sectorZ, fac->foundingDate, fac->expansionRate, fac->name.c_str(), factionName.c_str());
		return 0;
	} else {
		return luaL_error(L, "faction '%s' already added\n", facbld->fac->name.c_str());
	}
}

static int l_fac_gc(lua_State *L)
{
	FactionBuilder *facbld = l_fac_check_builder(L, 1);
	if (!facbld->registered) {
		delete facbld->fac;
		facbld->fac = 0;
	}
	return 0;
}

static luaL_Reg LuaFaction_meta[] = {
	{ "new",                       &l_fac_new },
	{ "description_short",         &l_fac_description_short },
	{ "description",               &l_fac_description },
	{ "govtype_weight",            &l_fac_govtype_weight },
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
	for (FactionIterator it = s_factions.begin(); it != s_factions.end(); ++it) {
		delete *it;
	}
	s_factions.clear();
	s_factions_indexes.clear();
}


Faction *Faction::GetFaction(const Uint32 index)
{
	assert( index < s_factions.size() );
	return s_factions[index];
}

const Uint32 Faction::GetNumFactions()
{
	return s_factions.size();
}

const Uint32 Faction::GetNearestFactionIndex(const SystemPath& sysPath)
{
	Uint32 ret_index = BAD_FACTION_IDX;

	/* firstly if this a custom StarSystem it may already have a faction assigned
	*/
	Sector sec(sysPath.sectorX, sysPath.sectorY, sysPath.sectorZ);	
	if (sec.m_systems[sysPath.systemIndex].customSys) {
		ret_index = sec.m_systems[sysPath.systemIndex].customSys->factionIdx;
	}
		
	/* if it didn't, or it wasn't a custom StarStystem, then we go ahead and assign it a faction allegiance like normal below...
	*/
	if (ret_index == BAD_FACTION_IDX) {

		// Iterate through all of the factions and find the one nearest to the system we're checking it against.
		const Faction *foundFaction = 0;
		double nearestDistance = HUGE_VAL;

		// get the current year
		// XXX: cannot access the PI::game->GetTime() method here as game is NULL when deserialised from save game -
		//	- I had hoped to use this to give a simple expanding spherical volume to each faction. Use 3200 as the-
		//	- base year, all factions should have come into existence prior to this date.
		const double current_year = 3200;//get_year(Pi::game->GetTime());

		// iterate
		for (Uint32 index = 0; index < s_factions.size(); ++index) {
			const Faction &fac = *s_factions[index];

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
	}

	return ret_index;
}

const Uint32 Faction::GetIndexOfFaction(const std::string factionName)
{
	FactionIndexes::iterator it = s_factions_indexes.find(factionName);

	if (it == s_factions_indexes.end()) return BAD_FACTION_IDX;
	else                                return it->second;
}

Polit::GovType Faction::PickGovType(MTRand &rand) const
{
	if( !govtype_weights.empty()) {
		// if we roll a number between one and the total weighting...
		int32_t roll = rand.Int32(1, govtype_weights_total);
		int32_t cumulativeWeight = 0;

		// ...the first govType with a cumulative weight >= the roll should be our pick
		GovWeightIterator it = govtype_weights.begin();
		while(roll > (cumulativeWeight + it->second)) { 
			cumulativeWeight += it->second;
			++it; 
		}
		return it->first;
	} else {
		return Polit::GOV_INVALID;
	}
}

/* If the si is negative, set the homeworld to our best shot at a system path
    pointing to a valid system, close to passed co-ordinates.

   Otherwise trust the caller, and just set the system path for the co-ordinates.

   Used by the Lua interface, to support autogenerated factions.
*/
void Faction::SetBestFitHomeworld(Sint32 x, Sint32 y, Sint32 z, Sint32 si, Uint32 bi)
{
	// if the current sector specified is empty move to a sector 
	// closer to the origin on the x-axis until we find one that isn't
	while (si < 0 && x != 0 ) {
		Sector sec(x,y,z);
		if (sec.m_systems.size() > 0 ) {
			si = 0;
		} else {
			if (x > 0) { --x; } else { ++x; };
		}
	}
	homeworld = SystemPath(x, y, z, si, bi);
}

Faction::Faction() :
	hasHomeworld(false),
	foundingDate(0.0),
	expansionRate(0.0)	
{
	govtype_weights_total = 0;
}

Faction::~Faction()
{
}
