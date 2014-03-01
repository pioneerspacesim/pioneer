// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Factions.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"

#include "LuaUtils.h"
#include "LuaVector.h"
#include "LuaFixed.h"
#include "LuaConstants.h"
#include "Polit.h"
#include "FileSystem.h"
#include "Lang.h"
#include "Pi.h"
#include <set>
#include <algorithm>

const Uint32 Faction::BAD_FACTION_IDX      = UINT_MAX;
const Color  Faction::BAD_FACTION_COLOUR   = Color(204,204,204,128);
const float  Faction::FACTION_BASE_ALPHA   = 0.40f;
const double Faction::FACTION_CURRENT_YEAR = 3200;


typedef std::vector<Faction*> FactionList;
typedef FactionList::iterator FactionIterator;
typedef const std::vector<Faction*> ConstFactionList;
typedef ConstFactionList::const_iterator ConstFactionIterator;
typedef std::map<std::string, Faction*> FactionMap;
typedef std::set<SystemPath>  HomeSystemSet;

static Faction       s_no_faction;    // instead of answering null, we often want to answer a working faction object for no faction

static FactionList       s_factions;
static FactionMap        s_factions_byName;
static HomeSystemSet     s_homesystems;
static FactionOctsapling s_spatial_index;
static bool             s_may_assign_factions;

// ------- Lua Faction Builder --------

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
	const Sint32 weight = luaL_checkinteger(L, 3);	// signed as we will need to compare with signed out of Random.Int32

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

	// search for home systems, first moving outward from the axes, then
	// if that didn't work moving inward toward them
	fac->hasHomeworld = true;
	fac->SetBestFitHomeworld(x, y, z, si, bi, +1);
	if (!fac->homeworld.HasValidSystem()) fac->SetBestFitHomeworld(x, y, z, si, bi, -1);

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
	const Uint32 probability = luaL_checkunsigned(L, 3);

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

	fac->colour = Color(r*255,g*255,b*255);

	lua_settop(L, 1);

	return 1;
}

#undef LFAC_FIELD_SETTER_FIXED
#undef LFAC_FIELD_SETTER_FLOAT
#undef LFAC_FIELD_SETTER_INT

static int l_fac_add_to_factions(lua_State *L)
{
	FactionBuilder *facbld = l_fac_check_builder(L, 1);

	const std::string factionName(luaL_checkstring(L, 2));

	if (!facbld->registered && !facbld->skip) {
		/* XXX maybe useful for debugging, leaving for now
		if (facbld->fac->hasHomeworld) {
			Output("l_fac_add_to_factions: added (%3d,%3d,%3d) f=%4.0f e=%2.2f '%s' [%s]\n"
				, fac->homeworld.sectorX, fac->homeworld.sectorY, fac->homeworld.sectorZ, fac->foundingDate, fac->expansionRate, fac->name.c_str(), factionName.c_str());
		}
		else {
			Output("l_fac_add_to_factions: added '%s' [%s]\n", fac->name.c_str(), factionName.c_str());
		}
		*/

		// add the faction to the various faction data structures
		s_factions.push_back(facbld->fac);
		s_factions_byName[facbld->fac->name] = facbld->fac;
		s_spatial_index.Add(facbld->fac);

		if (facbld->fac->hasHomeworld) s_homesystems.insert(facbld->fac->homeworld.SystemOnly());
		facbld->fac->idx = s_factions.size()-1;
		facbld->registered = true;

		return 0;
	} else if (facbld->skip) {
		/* XXX maybe useful for debugging, leaving for now
		Output("l_fac_add_to_factions: invalid homeworld, skipped (%3d,%3d,%3d) f=%4.0f e=%2.2f '%s' [%s]\n"
				, fac->homeworld.sectorX, fac->homeworld.sectorY, fac->homeworld.sectorZ, fac->foundingDate, fac->expansionRate, fac->name.c_str(), factionName.c_str());
		*/
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

	Output("Number of factions added: " SIZET_FMT "\n", s_factions.size());
	Faction::ClearHomeSectors();
	Pi::FlushCaches();    // clear caches of anything we used for faction generation
	s_may_assign_factions = true;
}

//static
void Faction::ClearHomeSectors()
{
	for (auto it = s_factions.begin(); it != s_factions.end(); ++it)
		(*it)->m_homesector.Reset();
}

//static
void Faction::SetHomeSectors()
{
	s_may_assign_factions = false;
	for (auto it = s_factions.begin(); it != s_factions.end(); ++it)
		if ((*it)->hasHomeworld)
			(*it)->m_homesector = Sector::cache.GetCached((*it)->homeworld);
	s_may_assign_factions = true;
	Sector::cache.AssignFactions();
}

void Faction::Uninit()
{
	for (FactionIterator it = s_factions.begin(); it != s_factions.end(); ++it) {
		delete *it;
	}
	s_factions.clear();
	s_factions_byName.clear();
}

// ------- Factions proper --------

Faction *Faction::GetFaction(const Uint32 index)
{
	PROFILE_SCOPED()
	assert( index < s_factions.size() );
	return s_factions[index];
}

Faction* Faction::GetFaction(const std::string& factionName)
{
	PROFILE_SCOPED()
	if (s_factions_byName.find(factionName) != s_factions_byName.end()) {
		return s_factions_byName[factionName];
	} else {
		return &s_no_faction;
	}
}

const Uint32 Faction::GetNumFactions()
{
	PROFILE_SCOPED()
	return s_factions.size();
}

bool Faction::MayAssignFactions()
{
	PROFILE_SCOPED()
	return s_may_assign_factions;
}

/*	Answer whether the faction both contains the sysPath, and has a homeworld
	closer than the passed distance.

	if it is, then the passed distance will also be updated to be the distance
	from the factions homeworld to the sysPath.
*/
const bool Faction::IsCloserAndContains(double& closestFactionDist, RefCountedPtr<const Sector> sec, Uint32 sysIndex)
{
	PROFILE_SCOPED()
	/*	Treat factions without homeworlds as if they are of effectively infinite radius,
		so every world is potentially within their borders, but also treat them as if
		they had a homeworld that was infinitely far away, so every other faction has
		a better claim.
	*/
	float distance = HUGE_VAL;
	bool  inside   = true;

	/*	Factions that have a homeworld... */
	if (hasHomeworld)
	{
		/* ...automatically gain the allegiance of worlds within the same sector... */
		if (sec->Contains(homeworld)) { distance = 0; }

		/* ...otherwise we need to calculate whether the world is inside the
		   the faction border, and how far away it is. */
		else {
			distance = Sector::DistanceBetween(GetHomeSector(), homeworld.systemIndex, sec, sysIndex);
			inside   = distance < Radius();
		}
	}

	/*	if the faction contains the world, and its homeworld is closer, then this faction
		wins, and we update the closestFactionDist */
	if (inside && (distance <= closestFactionDist)) {
		closestFactionDist = distance;
		return true;

	/* otherwise this isn't the faction we were looking for */
	} else {
		return false;
	}
}

Faction* Faction::GetNearestFaction(RefCountedPtr<const Sector> sec, Uint32 sysIndex)
{
	PROFILE_SCOPED()
	// firstly if this a custom StarSystem it may already have a faction assigned
	if (sec->m_systems[sysIndex].customSys && sec->m_systems[sysIndex].customSys->faction) {
		return sec->m_systems[sysIndex].customSys->faction;
	}

	// if it didn't, or it wasn't a custom StarStystem, then we go ahead and assign it a faction allegiance like normal below...
	Faction*    result             = &s_no_faction;
	double      closestFactionDist = HUGE_VAL;
	ConstFactionList& candidates   = s_spatial_index.CandidateFactions(sec, sysIndex);

	for (ConstFactionIterator it = candidates.begin(); it != candidates.end(); ++it) {
		if ((*it)->IsCloserAndContains(closestFactionDist, sec, sysIndex)) result = *it;
	}
	return result;
}

bool Faction::IsHomeSystem(const SystemPath& sysPath)
{
	PROFILE_SCOPED()
	return s_homesystems.find(sysPath.SystemOnly()) != s_homesystems.end();
}

const Color Faction::AdjustedColour(fixed population, bool inRange)
{
	PROFILE_SCOPED()
	Color result;
	result   = population == 0 ? BAD_FACTION_COLOUR : colour;
	result.a = population > 0  ? (FACTION_BASE_ALPHA + (M_E + (logf(population.ToFloat() / 1.25))) / ((2 * M_E) + FACTION_BASE_ALPHA)) * 255 : FACTION_BASE_ALPHA * 255;
	result.a = inRange         ? 255 : result.a;
	return result;
}

const Polit::GovType Faction::PickGovType(Random &rand) const
{
	PROFILE_SCOPED()
	if( !govtype_weights.empty()) {
		// if we roll a number between one and the total weighting...
		Sint32 roll = rand.Int32(1, govtype_weights_total);
		Sint32 cumulativeWeight = 0;

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
void Faction::SetBestFitHomeworld(Sint32 x, Sint32 y, Sint32 z, Sint32 si, Uint32 bi, Sint32 axisChange)
{
	PROFILE_SCOPED()
	// search for a home system until we either find one sutiable, hit one of the axes,
	// or hit the edge of inhabited space
	Uint32 i = 0;
	RefCountedPtr<StarSystem> sys;
	while (si < 0 && (abs(x) != 90 && abs(y) != 90 && abs(z) != 90)
		          && (    x  != 0  &&     y  !=  0 &&     z  != 0 )) {

		SystemPath path(x, y, z);
		// search for a suitable homeworld in the current sector
		assert(!s_may_assign_factions);
		RefCountedPtr<const Sector> sec = Sector::cache.GetCached(path);
		Sint32 candidateSi = 0;
		while (Uint32(candidateSi) < sec->m_systems.size()) {
			path.systemIndex = candidateSi;
			sys = StarSystemCache::GetCached(path);
			if (sys->HasSpaceStations()) {
				si = candidateSi;
				break;
			}
			candidateSi++;
		}

		// set the co-ordinates of the next sector to examine, cycling through x, y and z
		if      (si < 0 && i%3==0) { if (x >= 0) { x = x + axisChange; } else { x = x - axisChange; }; }
		else if (si < 0 && i%3==1) { if (y >= 0) { y = y + axisChange; } else { y = y - axisChange; }; }
		else if (si < 0 && i%3==2) { if (z >= 0) { z = z + axisChange; } else { z = z - axisChange; }; }
		i++;
	}
	homeworld = SystemPath(x, y, z, si);
}

RefCountedPtr<const Sector> Faction::GetHomeSector() {
	if (!m_homesector) // This will later be replaced by a Sector from the cache
		m_homesector = Sector::cache.GetCached(homeworld);
	return m_homesector;
}

Faction::Faction() :
	idx(BAD_FACTION_IDX),
	name(Lang::NO_CENTRAL_GOVERNANCE),
	hasHomeworld(false),
	foundingDate(0.0),
	expansionRate(0.0),
	colour(BAD_FACTION_COLOUR),
	m_homesector(0)
{
	PROFILE_SCOPED()
	govtype_weights_total = 0;
}

// ------ Factions Spatial Indexing ------

void FactionOctsapling::Add(Faction* faction)
{
	PROFILE_SCOPED()
	/*  The general principle here is to put the faction in every octbox cell that a system
	    that is a member of that faction could be in. This should let us cut the number
		of factions that have to be checked by GetNearestFaction, by eliminating right off
		all the those Factions that aren't in the same cell.

		As I'm just going for the quick performance win, I'm being very sloppy and
		treating a Faction as if it was a cube rather than a sphere. I'm also not even
		attempting to work out real faction boundaries for this.

		Obviously this all could be improved even without this Octsapling growing into
		a full Octree.

		This part happens at faction generation time so shouldn't be too performance
		critical
	*/
	RefCountedPtr<const Sector> sec = faction->GetHomeSector();

	/* only factions with homeworlds that are available at faction generation time can
	   be added to specific cells...
	*/
	if (faction->hasHomeworld && (faction->homeworld.systemIndex < sec->m_systems.size())) {
		/* calculate potential indexes for the octbox cells the faction needs to go into
		*/
		Sector::System sys = sec->m_systems[faction->homeworld.systemIndex];

		int xmin = BoxIndex(Sint32(sys.FullPosition().x - float((faction->Radius()))));
		int xmax = BoxIndex(Sint32(sys.FullPosition().x + float((faction->Radius()))));
		int ymin = BoxIndex(Sint32(sys.FullPosition().y - float((faction->Radius()))));
		int ymax = BoxIndex(Sint32(sys.FullPosition().y + float((faction->Radius()))));
		int zmin = BoxIndex(Sint32(sys.FullPosition().z - float((faction->Radius()))));
		int zmax = BoxIndex(Sint32(sys.FullPosition().z + float((faction->Radius()))));

		/* put the faction in all the octbox cells needed in a hideously inexact way that
		   will generate duplicates in each cell in many cases
		*/
		octbox[xmin][ymin][zmin].push_back(faction);  // 0,0,0
		octbox[xmax][ymin][zmin].push_back(faction);  // 1,0,0
		octbox[xmax][ymax][zmin].push_back(faction);  // 1,1,0
		octbox[xmax][ymax][zmax].push_back(faction);  // 1,1,1

		octbox[xmin][ymax][zmin].push_back(faction);  // 0,1,0
		octbox[xmin][ymax][zmax].push_back(faction);  // 0,1,1
		octbox[xmin][ymin][zmax].push_back(faction);  // 0,0,1
		octbox[xmax][ymin][zmax].push_back(faction);  // 1,0,1

		/* prune any duplicates from the octbox cells making things slightly saner
		*/
		PruneDuplicates(0,0,0);
		PruneDuplicates(1,0,0);
		PruneDuplicates(1,1,0);
		PruneDuplicates(1,1,1);

		PruneDuplicates(0,1,0);
		PruneDuplicates(0,1,1);
		PruneDuplicates(0,0,1);
		PruneDuplicates(1,0,1);

	} else {
	/* ...other factions, such as ones with no homeworlds, and more annoyingly ones
	   whose homeworlds don't exist yet because they're custom systems have to go in
	   *every* octbox cell
	*/
		octbox[0][0][0].push_back(faction);
		octbox[1][0][0].push_back(faction);
		octbox[1][1][0].push_back(faction);
		octbox[1][1][1].push_back(faction);

		octbox[0][1][0].push_back(faction);
		octbox[0][1][1].push_back(faction);
		octbox[0][0][1].push_back(faction);
		octbox[1][0][1].push_back(faction);
	}
}

void FactionOctsapling::PruneDuplicates(const int bx, const int by, const int bz)
{
	PROFILE_SCOPED()
	FactionList vec = octbox[bx][by][bz];
	octbox[bx][by][bz].erase(std::unique( octbox[bx][by][bz].begin(), octbox[bx][by][bz].end() ), octbox[bx][by][bz].end() );
}

const std::vector<Faction*>& FactionOctsapling::CandidateFactions(RefCountedPtr<const Sector> sec, Uint32 sysIndex)
{
	PROFILE_SCOPED()
	/* answer the factions that we've put in the same octobox cell as the one the
	   system would go in. This part happens every time we do GetNearest faction
	   so *is* performance criticale.e
	*/
	Sector::System sys = sec->m_systems[sysIndex];
	return octbox[BoxIndex(sys.sx)][BoxIndex(sys.sy)][BoxIndex(sys.sz)];
}
