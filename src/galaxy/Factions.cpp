// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Factions.h"
#include "galaxy/CustomSystem.h"
#include "galaxy/Economy.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"

#include "enum_table.h"

#include "FileSystem.h"
#include "GameSaveError.h"
#include "Lang.h"
#include "Pi.h"
#include "Polit.h"
#include "lua/LuaConstants.h"
#include "lua/LuaFixed.h"
#include "lua/LuaUtils.h"
#include "lua/LuaVector.h"
#include <algorithm>
#include <list>
#include <set>
#include <sstream>

const Uint32 Faction::BAD_FACTION_IDX = UINT_MAX;
const Color Faction::BAD_FACTION_COLOUR = Color(204, 204, 204, 128);
const float Faction::FACTION_BASE_ALPHA = 0.40f;
const double Faction::FACTION_CURRENT_YEAR = 3200;

//#define DUMP_FACTIONS
#ifdef DUMP_FACTIONS
const std::string SAVE_TARGET_DIR = "factions";
#endif

// ------- Lua Faction Builder --------

struct FactionBuilder {
	Faction *fac;
	bool registered;
	bool skip;
};

static const char LuaFaction_TypeName[] = "Faction";

static FactionsDatabase *s_activeFactionsDatabase = nullptr;

static FactionBuilder *l_fac_check_builder(lua_State *L, int idx)
{
	FactionBuilder *facbld = static_cast<FactionBuilder *>(
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

	FactionBuilder *facbld = static_cast<FactionBuilder *>(lua_newuserdata(L, sizeof(*facbld)));
	facbld->fac = new Faction(s_activeFactionsDatabase->GetGalaxy());
	facbld->registered = false;
	facbld->skip = false;
	luaL_setmetatable(L, LuaFaction_TypeName);

	facbld->fac->name = name;

	return 1;
}

#define LFAC_FIELD_SETTER_FIXED(luaname, fieldname)        \
	static int l_fac_##luaname(lua_State *L)               \
	{                                                      \
		Faction *fac = l_fac_check(L, 1);                  \
		const fixed *value = LuaFixed::CheckFromLua(L, 2); \
		fac->fieldname = *value;                           \
		lua_settop(L, 1);                                  \
		return 1;                                          \
	}

#define LFAC_FIELD_SETTER_FLOAT(luaname, fieldname) \
	static int l_fac_##luaname(lua_State *L)        \
	{                                               \
		Faction *fac = l_fac_check(L, 1);           \
		double value = luaL_checknumber(L, 2);      \
		fac->fieldname = value;                     \
		lua_settop(L, 1);                           \
		return 1;                                   \
	}

#define LFAC_FIELD_SETTER_INT(luaname, fieldname) \
	static int l_fac_##luaname(lua_State *L)      \
	{                                             \
		Faction *fac = l_fac_check(L, 1);         \
		int value = luaL_checkinteger(L, 2);      \
		fac->fieldname = value;                   \
		lua_settop(L, 1);                         \
		return 1;                                 \
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
	const Sint32 weight = luaL_checkinteger(L, 3); // signed as we will need to compare with signed out of Random.Int32

	if (g < Polit::GOV_RAND_MIN || g > Polit::GOV_RAND_MAX) {
		pi_lua_warn(L,
			"government type out of range: Faction{%s}:govtype_weight('%s', %d)",
			fac->name.c_str(), typeName, weight);
		return 0;
	}

	if (weight < 0) {
		pi_lua_warn(L,
			"weight must be a positive integer: Faction{%s}:govtype_weight('%s', %d)",
			fac->name.c_str(), typeName, weight);
		return 0;
	}

	fac->govtype_weights.push_back(std::make_pair(g, weight));
	fac->govtype_weights_total += weight;
	lua_settop(L, 1);

	return 1;
}

// sector(x,y,x) + system index + body index = location in a (custom?) system of homeworld
static int l_fac_homeworld(lua_State *L)
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

	facbld->skip = !fac->homeworld.HasValidSystem(); // wasn't a valid system
	lua_settop(L, 1);
	return 1;
}

LFAC_FIELD_SETTER_FLOAT(foundingDate, foundingDate)	  // date faction came into existence
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

//preferred police ship model
static int l_fac_police_ship(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	std::string police_ship = luaL_checkstring(L, 2);
	fac->police_ship = police_ship;
	lua_settop(L, 1);
	return 1;
}

//commodity legality
static int l_fac_illegal_goods_probability(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);
	std::string typeName = luaL_checkstring(L, 2);
	const GalacticEconomy::CommodityId e = GalacticEconomy::GetCommodityByName(typeName);
	const Uint32 probability = luaL_checkunsigned(L, 3);

	if (probability > 100) {
		pi_lua_warn(L,
			"argument (probability 0-100) out of range: Faction{%s}:IllegalGoodsProbability('%s', %d)",
			fac->name.c_str(), typeName.c_str(), probability);
		return 0;
	}

	if (e == GalacticEconomy::InvalidCommodityId) {
		pi_lua_warn(L, "Faction[%s]: Invalid commodity name %s passed to IllegalGoodsProbability.",
			fac->name.c_str(), typeName.c_str());
	}

	fac->commodity_legality[e] = probability;
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

	fac->colour = Color(r * 255, g * 255, b * 255);

	lua_settop(L, 1);

	return 1;
}

static int l_fac_claim(lua_State *L)
{
	Faction *fac = l_fac_check(L, 1);

	Sint32 sector_x = luaL_checkinteger(L, 2);
	Sint32 sector_y = luaL_checkinteger(L, 3);
	Sint32 sector_z = luaL_checkinteger(L, 4);

	SystemPath path(sector_x, sector_y, sector_z);

	path.systemIndex = -99; // magic number, claim whole sector

	if (lua_gettop(L) > 4) {
		path.systemIndex = luaL_checkinteger(L, 5);
	}
	fac->PushClaim(path);
	lua_settop(L, 1);
	return 1;
}

#ifdef DUMP_FACTIONS
static void ExportFactionToLua(const Faction *fac, const size_t index)
{
	time_t now;
	time(&now);
	char timeFormat[256];
	tm *now_tm = localtime(&now);
	strftime(timeFormat, 255, "%Y", now_tm);

	std::stringstream outstr;
	outstr << "-- Copyright © 2008-" << timeFormat << " Pioneer Developers. See AUTHORS.txt for details" << std::endl;
	outstr << "-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt" << std::endl;
	outstr << std::endl;
	outstr << "local f = Faction:new('" << fac->name << "')" << std::endl;
	outstr << "\t:description_short('" << fac->description_short << "')" << std::endl;
	outstr << "\t:description('" << fac->description << "')" << std::endl;
	if (fac->hasHomeworld) outstr << "\t:homeworld(" << fac->homeworld.sectorX << "," << fac->homeworld.sectorY << "," << fac->homeworld.sectorZ << "," << fac->homeworld.systemIndex << "," << fac->homeworld.bodyIndex << ")" << std::endl;
	outstr << "\t:foundingDate(" << fac->foundingDate << ")" << std::endl;
	outstr << "\t:expansionRate(" << fac->expansionRate << ")" << std::endl;
	if (!fac->military_name.empty()) outstr << "\t:military_name('" << fac->military_name << "')" << std::endl;
	if (!fac->police_name.empty()) outstr << "\t:police_name('" << fac->police_name << "')" << std::endl;
	if (!fac->police_ship.empty()) outstr << "\t:police_ship('" << fac->police_ship << "')" << std::endl;
	outstr << "\t:colour(" << float(fac->colour.r) / 255.f << "," << float(fac->colour.g) / 255.f << "," << float(fac->colour.b) / 255.f << ")" << std::endl;
	outstr << std::endl;

	for (size_t i = 0; i < fac->govtype_weights.size(); i++) {
		const Polit::GovType gt = fac->govtype_weights[i].first;
		for (Uint32 j = 0; ENUM_PolitGovType[j].name != 0; j++) {
			if (ENUM_PolitGovType[j].value == gt) {
				outstr << "f:govtype_weight('" << ENUM_PolitGovType[j].name << "',\t\t" << fac->govtype_weights[i].second << ")" << std::endl;
			}
		}
	}
	outstr << std::endl;

	for (auto m : fac->commodity_legality) {
		const GalacticEconomy::CommodityId gec = m.first;
		outstr << "f:illegal_goods_probability('" << GalacticEconomy::GetCommodityById(gec).name << "',\t\t" << m.second << ")" << std::endl;
	}
	outstr << std::endl;

	outstr << "f:add_to_factions('" << fac->name << "')\n\n"
		   << std::endl;

	FILE *f = nullptr;
	FileSystem::FileSourceFS newFS(FileSystem::GetDataDir());

	//if (!bInPlace)
	{
		if (!FileSystem::userFiles.MakeDirectory(SAVE_TARGET_DIR))
			throw CouldNotOpenFileException();

		char numStr[6];
		snprintf(numStr, 6, "%000.3u_", index);
		std::string filenameOut = FileSystem::JoinPathBelow(SAVE_TARGET_DIR, std::string(numStr) + fac->name + std::string(".lua"));
		std::replace(filenameOut.begin(), filenameOut.end(), ' ', '_'); // replace all 'x' to 'y'
		f = FileSystem::userFiles.OpenWriteStream(filenameOut);
		printf("Save file (%s)\n", filenameOut.c_str());
		if (!f) throw CouldNotOpenFileException();
	}
	//else {
	//	f = newFS.OpenWriteStream(savepath + SGM_EXTENSION);
	//	if (!f) throw CouldNotOpenFileException();
	//}
	const size_t nwritten = fwrite(outstr.str().c_str(), outstr.str().size(), 1, f);
	fclose(f);

	if (nwritten != 1) throw CouldNotWriteToFileException();
}
#endif

#undef LFAC_FIELD_SETTER_FIXED
#undef LFAC_FIELD_SETTER_FLOAT
#undef LFAC_FIELD_SETTER_INT

static int l_fac_add_to_factions(lua_State *L)
{
	FactionBuilder *facbld = l_fac_check_builder(L, 1);

	//const std::string factionName(luaL_checkstring(L, 2));

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

		// if we haven't defined any commodity legality information,
		// load default legality from GalacticEconomy commodity data
		if (!facbld->fac->commodity_legality.size()) {
			for (const auto &commodity : GalacticEconomy::Commodities()) {
				if (commodity.default_legality < fixed(1, 1)) {
					fixed legal_percent = (fixed(1, 1) - commodity.default_legality) * fixed(100, 1);
					facbld->fac->commodity_legality[commodity.id] = legal_percent.ToDouble();
				}
			}
		}

		// add the faction to the various faction data structures
		s_activeFactionsDatabase->AddFaction(facbld->fac);
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
	{ "new", &l_fac_new },
	{ "description_short", &l_fac_description_short },
	{ "description", &l_fac_description },
	{ "govtype_weight", &l_fac_govtype_weight },
	{ "homeworld", &l_fac_homeworld },
	{ "foundingDate", &l_fac_foundingDate },
	{ "expansionRate", &l_fac_expansionRate },
	{ "military_name", &l_fac_military_name },
	{ "police_name", &l_fac_police_name },
	{ "police_ship", &l_fac_police_ship },
	{ "illegal_goods_probability", &l_fac_illegal_goods_probability },
	{ "colour", &l_fac_colour },
	{ "add_to_factions", &l_fac_add_to_factions },
	{ "claim", &l_fac_claim }, // claim possession of a starsystem or sector
	{ "__gc", &l_fac_gc },
	{ 0, 0 }
};

// ------ FactionsDatabase ------

FactionsDatabase::~FactionsDatabase()
{
	if (m_initialized) {
		m_initialized = false;
		for (FactionIterator it = m_factions.begin(); it != m_factions.end(); ++it) {
			delete *it;
		}
		m_factions.clear();
		m_factions_byName.clear();
	}
}

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

void FactionsDatabase::Init()
{
	assert(!s_activeFactionsDatabase);
	s_activeFactionsDatabase = this;

	lua_State *L = luaL_newstate();
	LUA_DEBUG_START(L);

	pi_lua_open_standard_base(L);

	LuaConstants::Register(L);

	LUA_DEBUG_CHECK(L, 0);

	RegisterFactionsAPI(L);

	LUA_DEBUG_CHECK(L, 0);
	pi_lua_dofile_recursive(L, m_factionDirectory);

	LUA_DEBUG_END(L, 0);
	lua_close(L);

	Output("Number of factions added: " SIZET_FMT "\n", m_factions.size());
	ClearHomeSectors();
	m_galaxy->FlushCaches(); // clear caches of anything we used for faction generation
	while (!m_missingFactionsMap.empty()) {
		const std::string &factionName = m_missingFactionsMap.begin()->first;
		std::list<CustomSystem *> &csl = m_missingFactionsMap.begin()->second;
		while (!csl.empty()) {
			CustomSystem *cs = csl.front();
			// FIXME: How to signal missing faction?
			fprintf(stderr, "Custom system %s referenced unknown faction %s\n", cs->name.c_str(), factionName.c_str());
			csl.pop_front();
		}
		m_missingFactionsMap.erase(m_missingFactionsMap.begin());
	}
	m_initialized = true;
	s_activeFactionsDatabase = nullptr;
}

void FactionsDatabase::PostInit()
{
	assert(m_initialized);
	assert(m_galaxy->IsInitialized());
	SetHomeSectors();

#ifdef DUMP_FACTIONS // useful for dumping the factions from an autogenerated script
	for (size_t i = 0; i < m_factions.size(); i++)
		ExportFactionToLua(m_factions[i], i);
#endif
}

void FactionsDatabase::ClearHomeSectors()
{
	for (auto it = m_factions.begin(); it != m_factions.end(); ++it)
		(*it)->m_homesector.Reset();
}

void FactionsDatabase::SetHomeSectors()
{
	m_may_assign_factions = false;
	for (auto it = m_factions.begin(); it != m_factions.end(); ++it)
		if ((*it)->hasHomeworld)
			(*it)->m_homesector = m_galaxy->GetSector((*it)->homeworld);
	m_may_assign_factions = true;
}

bool FactionsDatabase::IsInitialized() const
{
	return m_initialized;
}

void FactionsDatabase::RegisterCustomSystem(CustomSystem *cs, const std::string &factionName)
{
	m_missingFactionsMap[factionName].push_back(cs);
}

void FactionsDatabase::AddFaction(Faction *faction)
{
	// add the faction to the various faction data structures
	m_factions.push_back(faction);
	m_factions_byName[faction->name] = faction;
	auto it = m_missingFactionsMap.find(faction->name);
	if (it != m_missingFactionsMap.end()) {
		std::list<CustomSystem *> &csl = it->second;
		for (CustomSystem *cs : csl) {
			cs->faction = faction;
		}
		m_missingFactionsMap.erase(it);
	}
	m_spatial_index.Add(faction);

	if (faction->hasHomeworld) m_homesystems.insert(faction->homeworld.SystemOnly());
	faction->idx = m_factions.size() - 1;
}

const Faction *FactionsDatabase::GetFaction(const Uint32 index) const
{
	PROFILE_SCOPED()
	assert(index < m_factions.size());
	return m_factions[index];
}

const Faction *FactionsDatabase::GetFaction(const std::string &factionName) const
{
	PROFILE_SCOPED()
	auto it = m_factions_byName.find(factionName);
	if (it != m_factions_byName.end()) {
		return it->second;
	} else {
		return &m_no_faction;
	}
}

Uint32 FactionsDatabase::GetNumFactions() const
{
	PROFILE_SCOPED()
	return m_factions.size();
}

bool FactionsDatabase::MayAssignFactions() const
{
	PROFILE_SCOPED()
	return m_may_assign_factions;
}

const Faction *FactionsDatabase::GetNearestClaimant(const Sector::System *sys) const
{
	PROFILE_SCOPED()
	// firstly if this a custom StarSystem it may already have a faction assigned
	if (sys->GetCustomSystem() && sys->GetCustomSystem()->faction) {
		return sys->GetCustomSystem()->faction;
	}

	// if it didn't, or it wasn't a custom StarStystem, then we go ahead and assign it a faction allegiance like normal below...
	const Faction *result = &m_no_faction;
	double closestFactionDist = HUGE_VAL;
	ConstFactionList &candidates = m_spatial_index.CandidateFactions(sys);

	for (ConstFactionIterator it = candidates.begin(); it != candidates.end(); ++it) {
		if ((*it)->IsClaimed(sys->GetPath()))
			return *it; // this is a very specific claim, no further checks for distance from another factions homeworld is needed.
		if ((*it)->IsCloserAndContains(closestFactionDist, sys))
			result = *it;
	}
	return result;
}

bool FactionsDatabase::IsHomeSystem(const SystemPath &sysPath) const
{
	return m_homesystems.find(sysPath.SystemOnly()) != m_homesystems.end();
}

// ------- Factions proper --------

bool Faction::IsClaimed(SystemPath path) const
{
	// check the factions list of claimed systems/sectors, if there is one
	SystemPath sector = path;
	sector.systemIndex = -99;

	for (auto clam = m_ownedsystemlist.begin(); clam != m_ownedsystemlist.end(); clam++) {
		if (*clam == sector || *clam == path)
			return true;
	}
	return false;
}

/*	Answer whether the faction both contains the sysPath, and has a homeworld
	closer than the passed distance.

	if it is, then the passed distance will also be updated to be the distance
	from the factions homeworld to the sysPath.
*/
bool Faction::IsCloserAndContains(double &closestFactionDist, const Sector::System *sys) const
{
	// PROFILE_SCOPED() // verbose profiling, this gets called a lot
	/*	Treat factions without homeworlds as if they are of effectively infinite radius,
		so every world is potentially within their borders, but also treat them as if
		they had a homeworld that was infinitely far away, so every other faction has
		a better claim.
	*/
	float distance = HUGE_VAL;
	bool inside = true;

	/*	Factions that have a homeworld... */
	if (hasHomeworld) {
		/* ...automatically gain the allegiance of worlds within the same sector... */
		if (sys->InSameSector(homeworld)) {
			distance = 0;
		}

		/* ...otherwise we need to calculate whether the world is inside the
		   the faction border, and how far away it is. */
		else {
			RefCountedPtr<const Sector> homeSec = GetHomeSector();
			const Sector::System *homeSys = &homeSec->m_systems[homeworld.systemIndex];
			distance = Sector::System::DistanceBetween(homeSys, sys);
			inside = distance < Radius();
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

Color Faction::AdjustedColour(fixed population, bool inRange) const
{
	PROFILE_SCOPED()
	Color result;
	// Unexplored: population = -1, Uninhabited: population = 0.
	result = population <= 0 ? BAD_FACTION_COLOUR : colour;
	result.a = population > 0 ? (FACTION_BASE_ALPHA + (M_E + (logf(population.ToFloat() / 1.25))) / ((2 * M_E) + FACTION_BASE_ALPHA)) * 255 : FACTION_BASE_ALPHA * 255;
	result.a = inRange ? 255 : result.a;
	return result;
}

Polit::GovType Faction::PickGovType(Random &rand) const
{
	PROFILE_SCOPED()
	if (!govtype_weights.empty()) {
		// if we roll a number between one and the total weighting...
		Sint32 roll = rand.Int32(1, govtype_weights_total);
		Sint32 cumulativeWeight = 0;

		// ...the first govType with a cumulative weight >= the roll should be our pick
		GovWeightIterator it = govtype_weights.begin();
		while (roll > (cumulativeWeight + it->second)) {
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
	while (si < 0 && (abs(x) != 90 && abs(y) != 90 && abs(z) != 90) && (x != 0 && y != 0 && z != 0)) {

		SystemPath path(x, y, z);
		// search for a suitable homeworld in the current sector
		RefCountedPtr<const Sector> sec = m_galaxy->GetSector(path);
		Sint32 candidateSi = 0;
		while (Uint32(candidateSi) < sec->m_systems.size()) {
			path.systemIndex = candidateSi;
			sys = m_galaxy->GetStarSystem(path);
			if (sys->HasSpaceStations()) {
				si = candidateSi;
				std::map<SystemPath, int> stationCount;
				for (auto station : sys->GetSpaceStations()) {
					if (stationCount.find(station->GetParent()->GetPath()) == stationCount.end()) {
						// new parent
						stationCount[station->GetParent()->GetPath()] = 1;
					} else {
						// increment count
						stationCount[station->GetParent()->GetPath()]++;
					}
				}
				Uint32 candidateBi = 0;
				Sint32 candidateCount = 0;
				for (auto count : stationCount) {
					if (candidateCount < count.second) {
						candidateBi = count.first.bodyIndex;
						candidateCount = count.second;
					}
				}
				bi = candidateBi;
				break;
			}
			candidateSi++;
		}

		// set the co-ordinates of the next sector to examine, cycling through x, y and z
		if (si < 0 && i % 3 == 0) {
			if (x >= 0) {
				x = x + axisChange;
			} else {
				x = x - axisChange;
			};
		} else if (si < 0 && i % 3 == 1) {
			if (y >= 0) {
				y = y + axisChange;
			} else {
				y = y - axisChange;
			};
		} else if (si < 0 && i % 3 == 2) {
			if (z >= 0) {
				z = z + axisChange;
			} else {
				z = z - axisChange;
			};
		}
		i++;
	}
	homeworld = SystemPath(x, y, z, si, bi);
}

RefCountedPtr<const Sector> Faction::GetHomeSector() const
{
	if (!m_homesector) // This will later be replaced by a Sector from the cache
		m_homesector = m_galaxy->GetSector(homeworld);
	return m_homesector;
}

Faction::Faction(Galaxy *galaxy) :
	idx(BAD_FACTION_IDX),
	name(Lang::NO_CENTRAL_GOVERNANCE),
	hasHomeworld(false),
	foundingDate(0.0),
	expansionRate(0.0),
	colour(BAD_FACTION_COLOUR),
	m_galaxy(galaxy),
	m_homesector(0)
{
	govtype_weights_total = 0;
}

// ------ Factions Spatial Indexing ------

void FactionsDatabase::Octsapling::Add(const Faction *faction)
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

		int xmin = BoxIndex(Sint32(sys.GetFullPosition().x - float((faction->Radius()))));
		int xmax = BoxIndex(Sint32(sys.GetFullPosition().x + float((faction->Radius()))));
		int ymin = BoxIndex(Sint32(sys.GetFullPosition().y - float((faction->Radius()))));
		int ymax = BoxIndex(Sint32(sys.GetFullPosition().y + float((faction->Radius()))));
		int zmin = BoxIndex(Sint32(sys.GetFullPosition().z - float((faction->Radius()))));
		int zmax = BoxIndex(Sint32(sys.GetFullPosition().z + float((faction->Radius()))));

		/* put the faction in all the octbox cells needed in a hideously inexact way that
		   will generate duplicates in each cell in many cases
		*/
		octbox[xmin][ymin][zmin].push_back(faction); // 0,0,0
		octbox[xmax][ymin][zmin].push_back(faction); // 1,0,0
		octbox[xmax][ymax][zmin].push_back(faction); // 1,1,0
		octbox[xmax][ymax][zmax].push_back(faction); // 1,1,1

		octbox[xmin][ymax][zmin].push_back(faction); // 0,1,0
		octbox[xmin][ymax][zmax].push_back(faction); // 0,1,1
		octbox[xmin][ymin][zmax].push_back(faction); // 0,0,1
		octbox[xmax][ymin][zmax].push_back(faction); // 1,0,1

		/* prune any duplicates from the octbox cells making things slightly saner
		*/
		PruneDuplicates(0, 0, 0);
		PruneDuplicates(1, 0, 0);
		PruneDuplicates(1, 1, 0);
		PruneDuplicates(1, 1, 1);

		PruneDuplicates(0, 1, 0);
		PruneDuplicates(0, 1, 1);
		PruneDuplicates(0, 0, 1);
		PruneDuplicates(1, 0, 1);

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

void FactionsDatabase::Octsapling::PruneDuplicates(const int bx, const int by, const int bz)
{
	octbox[bx][by][bz].erase(std::unique(octbox[bx][by][bz].begin(), octbox[bx][by][bz].end()), octbox[bx][by][bz].end());
}

const std::vector<const Faction *> &FactionsDatabase::Octsapling::CandidateFactions(const Sector::System *sys) const
{
	PROFILE_SCOPED()
	/* answer the factions that we've put in the same octobox cell as the one the
	   system would go in. This part happens every time we do GetNearest faction
	   so *is* performance criticale.e
	*/
	return octbox[BoxIndex(sys->sx)][BoxIndex(sys->sy)][BoxIndex(sys->sz)];
}
