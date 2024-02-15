// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StarSystem.h"

#include "Galaxy.h"
#include "JsonUtils.h"
#include "Sector.h"

#include "EnumStrings.h"
#include "GameSaveError.h"
#include "Lang.h"
#include "Orbit.h"
#include "StringF.h"
#include "enum_table.h"
#include "galaxy/Economy.h"
#include "lua/LuaEvent.h"
#include "core/StringUtils.h"
#include "profiler/Profiler.h"

#include <SDL_stdinc.h>
#include <algorithm>
#include <map>
#include <string>

//#define DEBUG_DUMP

namespace {
	bool InvalidSystemNameChar(char c)
	{
		return !(
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9'));
	}
} // namespace

// indexed by enum type turd
const Color StarSystem::starColors[] = {
	{ 0, 0, 0 },	   // gravpoint
	{ 128, 0, 0 },	   // brown dwarf
	{ 102, 102, 204 }, // white dwarf
	{ 255, 51, 0 },	   // M
	{ 255, 153, 26 },  // K
	{ 255, 255, 102 }, // G
	{ 255, 255, 204 }, // F
	{ 255, 255, 255 }, // A
	{ 178, 178, 255 }, // B
	{ 255, 178, 255 }, // O
	{ 255, 51, 0 },	   // M Giant
	{ 255, 153, 26 },  // K Giant
	{ 255, 255, 102 }, // G Giant
	{ 255, 255, 204 }, // F Giant
	{ 255, 255, 255 }, // A Giant
	{ 178, 178, 255 }, // B Giant
	{ 255, 178, 255 }, // O Giant
	{ 255, 51, 0 },	   // M Super Giant
	{ 255, 153, 26 },  // K Super Giant
	{ 255, 255, 102 }, // G Super Giant
	{ 255, 255, 204 }, // F Super Giant
	{ 255, 255, 255 }, // A Super Giant
	{ 178, 178, 255 }, // B Super Giant
	{ 255, 178, 255 }, // O Super Giant
	{ 255, 51, 0 },	   // M Hyper Giant
	{ 255, 153, 26 },  // K Hyper Giant
	{ 255, 255, 102 }, // G Hyper Giant
	{ 255, 255, 204 }, // F Hyper Giant
	{ 255, 255, 255 }, // A Hyper Giant
	{ 178, 178, 255 }, // B Hyper Giant
	{ 255, 178, 255 }, // O Hyper Giant
	{ 255, 51, 0 },	   // Red/M Wolf Rayet Star
	{ 178, 178, 255 }, // Blue/B Wolf Rayet Star
	{ 255, 178, 255 }, // Purple-Blue/O Wolf Rayet Star
	{ 76, 178, 76 },   // Stellar Blackhole
	{ 51, 230, 51 },   // Intermediate mass Black-hole
	{ 0, 255, 0 }	   // Super massive black hole
};

// indexed by enum type turd
const Color StarSystem::starRealColors[] = {
	{ 0, 0, 0 },	   // gravpoint
	{ 128, 0, 0 },	   // brown dwarf
	{ 255, 255, 255 }, // white dwarf
	{ 255, 128, 51 },  // M
	{ 255, 255, 102 }, // K
	{ 255, 255, 242 }, // G
	{ 255, 255, 255 }, // F
	{ 255, 255, 255 }, // A
	{ 204, 204, 255 }, // B
	{ 255, 204, 255 }, // O
	{ 255, 128, 51 },  // M Giant
	{ 255, 255, 102 }, // K Giant
	{ 255, 255, 242 }, // G Giant
	{ 255, 255, 255 }, // F Giant
	{ 255, 255, 255 }, // A Giant
	{ 204, 204, 255 }, // B Giant
	{ 255, 204, 255 }, // O Giant
	{ 255, 128, 51 },  // M Super Giant
	{ 255, 255, 102 }, // K Super Giant
	{ 255, 255, 242 }, // G Super Giant
	{ 255, 255, 255 }, // F Super Giant
	{ 255, 255, 255 }, // A Super Giant
	{ 204, 204, 255 }, // B Super Giant
	{ 255, 204, 255 }, // O Super Giant
	{ 255, 128, 51 },  // M Hyper Giant
	{ 255, 255, 102 }, // K Hyper Giant
	{ 255, 255, 242 }, // G Hyper Giant
	{ 255, 255, 255 }, // F Hyper Giant
	{ 255, 255, 255 }, // A Hyper Giant
	{ 204, 204, 255 }, // B Hyper Giant
	{ 255, 204, 255 }, // O Hyper Giant
	{ 255, 153, 153 }, // M WF
	{ 204, 204, 255 }, // B WF
	{ 255, 204, 255 }, // O WF
	{ 22, 0, 24 },	   // small Black hole
	{ 16, 0, 20 },	   // med BH
	{ 10, 0, 16 }	   // massive BH
};

const double StarSystem::starLuminosities[] = {
	0,
	0.0003,	  // brown dwarf
	0.1,	  // white dwarf
	0.08,	  // M0
	0.38,	  // K0
	1.2,	  // G0
	5.1,	  // F0
	24.0,	  // A0
	100.0,	  // B0
	200.0,	  // O5
	1000.0,	  // M0 Giant
	2000.0,	  // K0 Giant
	4000.0,	  // G0 Giant
	6000.0,	  // F0 Giant
	8000.0,	  // A0 Giant
	9000.0,	  // B0 Giant
	12000.0,  // O5 Giant
	12000.0,  // M0 Super Giant
	14000.0,  // K0 Super Giant
	18000.0,  // G0 Super Giant
	24000.0,  // F0 Super Giant
	30000.0,  // A0 Super Giant
	50000.0,  // B0 Super Giant
	100000.0, // O5 Super Giant
	125000.0, // M0 Hyper Giant
	150000.0, // K0 Hyper Giant
	175000.0, // G0 Hyper Giant
	200000.0, // F0 Hyper Giant
	200000.0, // A0 Hyper Giant
	200000.0, // B0 Hyper Giant
	200000.0, // O5 Hyper Giant
	50000.0,  // M WF
	100000.0, // B WF
	200000.0, // O WF
	0.0003,	  // Stellar Black hole
	0.00003,  // IM Black hole
	0.000003, // Supermassive Black hole
};

const float StarSystem::starScale[] = {
	// Used in sector view
	0,
	0.6f, // brown dwarf
	0.5f, // white dwarf
	0.7f, // M
	0.8f, // K
	0.8f, // G
	0.9f, // F
	1.0f, // A
	1.1f, // B
	1.1f, // O
	1.3f, // M Giant
	1.2f, // K G
	1.2f, // G G
	1.2f, // F G
	1.1f, // A G
	1.1f, // B G
	1.2f, // O G
	1.8f, // M Super Giant
	1.6f, // K SG
	1.5f, // G SG
	1.5f, // F SG
	1.4f, // A SG
	1.3f, // B SG
	1.3f, // O SG
	2.5f, // M Hyper Giant
	2.2f, // K HG
	2.2f, // G HG
	2.1f, // F HG
	2.1f, // A HG
	2.0f, // B HG
	1.9f, // O HG
	1.1f, // M WF
	1.3f, // B WF
	1.6f, // O WF
	1.0f, // Black hole
	2.5f, // Intermediate-mass blackhole
	4.0f  // Supermassive blackhole
};

SystemBody *StarSystem::GetBodyByPath(const SystemPath &path) const
{
	PROFILE_SCOPED()
	assert(m_path.IsSameSystem(path));
	assert(path.IsBodyPath());
	assert(path.bodyIndex < m_bodies.size());

	return m_bodies[path.bodyIndex].Get();
}

SystemPath StarSystem::GetPathOf(const SystemBody *sbody) const
{
	return sbody->GetPath();
}

/*
 * As my excellent comrades have pointed out, choices that depend on floating
 * point crap will result in different universes on different platforms.
 *
 * We must be sneaky and avoid floating point in these places.
 */
StarSystem::StarSystem(const SystemPath &path, RefCountedPtr<Galaxy> galaxy, StarSystemCache *cache, Random &rand) :
	m_galaxy(galaxy),
	m_path(path.SystemOnly()),
	m_numStars(0),
	m_isCustom(false),
	m_faction(nullptr),
	m_explored(eEXPLORED_AT_START),
	m_exploredTime(0.0),
	m_econType(GalacticEconomy::InvalidEconomyId),
	m_seed(0),
	m_tradeLevel(GalacticEconomy::Commodities().size() + 1, 0),
	m_commodityLegal(GalacticEconomy::Commodities().size() + 1, true),
	m_cache(cache)
{
}

StarSystem::GeneratorAPI::GeneratorAPI(const SystemPath &path, RefCountedPtr<Galaxy> galaxy, StarSystemCache *cache, Random &rand) :
	StarSystem(path, galaxy, cache, rand) {}

#ifdef DEBUG_DUMP
struct thing_t {
	SystemBody *obj;
	vector3d pos;
	vector3d vel;
};
void StarSystem::Dump()
{
	std::vector<SystemBody *> obj_stack;
	std::vector<vector3d> pos_stack;
	std::vector<thing_t> output;

	SystemBody *obj = m_rootBody;
	vector3d pos = vector3d(0.0);

	while (obj) {
		vector3d p2 = pos;
		if (obj->m_parent) {
			p2 = pos + obj->m_orbit.OrbitalPosAtTime(1.0);
			pos = pos + obj->m_orbit.OrbitalPosAtTime(0.0);
		}

		if ((obj->GetType() != SystemBody::TYPE_GRAVPOINT) &&
			(obj->GetSuperType() != SystemBody::SUPERTYPE_STARPORT)) {
			struct thing_t t;
			t.obj = obj;
			t.pos = pos;
			t.vel = (p2 - pos);
			output.push_back(t);
		}
		for (std::vector<SystemBody *>::iterator i = obj->m_children.begin();
			 i != obj->m_children.end(); ++i) {
			obj_stack.push_back(*i);
			pos_stack.push_back(pos);
		}
		if (obj_stack.size() == 0) break;
		pos = pos_stack.back();
		obj = obj_stack.back();
		pos_stack.pop_back();
		obj_stack.pop_back();
	}

	FILE *f = fopen("starsystem.dump", "w");
	fprintf(f, SIZET_FMT " bodies\n", output.size());
	fprintf(f, "0 steps\n");
	for (std::vector<thing_t>::iterator i = output.begin();
		 i != output.end(); ++i) {
		fprintf(f, "B:%lf,%lf:%lf,%lf,%lf,%lf:%lf:%d:%lf,%lf,%lf\n",
			(*i).pos.x, (*i).pos.y, (*i).pos.z,
			(*i).vel.x, (*i).vel.y, (*i).vel.z,
			(*i).obj->GetMass(), 0,
			1.0, 1.0, 1.0);
	}
	fclose(f);
	Output("Junk dumped to starsystem.dump\n");
}
#endif /* DEBUG_DUMP */

void StarSystem::MakeShortDescription()
{
	PROFILE_SCOPED()
	if (GetExplored() == StarSystem::eUNEXPLORED)
		SetShortDesc(Lang::UNEXPLORED_SYSTEM_NO_DATA);

	else if (GetExplored() == StarSystem::eEXPLORED_BY_PLAYER)
		SetShortDesc(stringf(Lang::RECENTLY_EXPLORED_SYSTEM, formatarg("date", format_date_only(GetExploredTime()))));

	/* Total population is in billions */
	else if (GetTotalPop() == 0) {
		SetShortDesc(Lang::SMALL_SCALE_PROSPECTING_NO_SETTLEMENTS);
	} else if (GetTotalPop() < fixed(1, 10)) {
		SetShortDesc(Lang::GetCore().Get(
			GalacticEconomy::GetEconomyById(GetEconType()).l10n_key.small));
	} else if (GetTotalPop() < fixed(1, 2)) {
		SetShortDesc(Lang::GetCore().Get(
			GalacticEconomy::GetEconomyById(GetEconType()).l10n_key.medium));
	} else if (GetTotalPop() < fixed(5, 1)) {
		SetShortDesc(Lang::GetCore().Get(
			GalacticEconomy::GetEconomyById(GetEconType()).l10n_key.large));
	} else {
		SetShortDesc(Lang::GetCore().Get(
			GalacticEconomy::GetEconomyById(GetEconType()).l10n_key.huge));
	}
}

void StarSystem::ExploreSystem(double time)
{
	if (m_explored != eUNEXPLORED)
		return;
	m_explored = eEXPLORED_BY_PLAYER;
	m_exploredTime = time;
	RefCountedPtr<Sector> sec = m_galaxy->GetMutableSector(m_path);
	Sector::System &secsys = sec->m_systems[m_path.systemIndex];
	secsys.SetExplored(m_explored, m_exploredTime);
	MakeShortDescription();
	LuaEvent::Queue("onSystemExplored", this);
}

StarSystem::~StarSystem()
{
	PROFILE_SCOPED()
	// clear parent and children pointers. someone (Lua) might still have a
	// reference to things that are about to be deleted
	m_rootBody->Orphan();
	if (m_cache)
		m_cache->RemoveFromAttic(m_path);
}

void StarSystem::ToJson(Json &jsonObj, StarSystem *s)
{
	if (s) {
		Json starSystemObj({}); // Create JSON object to contain star system data.
		starSystemObj["sector_x"] = s->m_path.sectorX;
		starSystemObj["sector_y"] = s->m_path.sectorY;
		starSystemObj["sector_z"] = s->m_path.sectorZ;
		starSystemObj["system_index"] = s->m_path.systemIndex;
		jsonObj["star_system"] = starSystemObj; // Add star system object to supplied object.
	}
}

RefCountedPtr<StarSystem> StarSystem::FromJson(RefCountedPtr<Galaxy> galaxy, const Json &jsonObj)
{
	try {
		Json starSystemObj = jsonObj["star_system"];

		int sec_x = starSystemObj["sector_x"];
		int sec_y = starSystemObj["sector_y"];
		int sec_z = starSystemObj["sector_z"];
		int sys_idx = starSystemObj["system_index"];

		return galaxy->GetStarSystem(SystemPath(sec_x, sec_y, sec_z, sys_idx));
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

std::string StarSystem::ExportBodyToLua(FILE *f, SystemBody *body)
{
	const int multiplier = 10000;

	// strip characters that will not work in Lua
	std::string code_name = body->GetName();
	std::transform(code_name.begin(), code_name.end(), code_name.begin(), ::tolower);
	code_name.erase(remove_if(code_name.begin(), code_name.end(), InvalidSystemNameChar), code_name.end());

	// find the body type index so we can lookup the name
	const char *pBodyTypeName = nullptr;
	for (int bodyTypeIdx = 0; ENUM_BodyType[bodyTypeIdx].name != 0; bodyTypeIdx++) {
		if (ENUM_BodyType[bodyTypeIdx].value == int(body->GetType())) {
			pBodyTypeName = ENUM_BodyType[bodyTypeIdx].name;
			break;
		}
	}

	if (body->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
		fprintf(f,
			"local %s = CustomSystemBody:new(\"%s\", '%s')\n"
			"\t:latitude(math.deg2rad(%.1f))\n"
			"\t:longitude(math.deg2rad(%.1f))\n",

			code_name.c_str(),
			body->GetName().c_str(), pBodyTypeName,
			body->m_inclination.ToDouble() * 180 / M_PI,
			body->m_orbitalOffset.ToDouble() * 180 / M_PI);
	} else {
		fprintf(f,
			"local %s = CustomSystemBody:new(\"%s\", '%s')\n"
			"\t:radius(f(%d,%d))\n"
			"\t:mass(f(%d,%d))\n",
			code_name.c_str(),
			body->GetName().c_str(), pBodyTypeName,
			int(round(body->GetRadiusAsFixed().ToDouble() * multiplier)), multiplier,
			int(round(body->GetMassAsFixed().ToDouble() * multiplier)), multiplier);

		if (body->GetType() != SystemBody::TYPE_GRAVPOINT)
			fprintf(f,
				"\t:seed(%u)\n"
				"\t:temp(%d)\n"
				"\t:semi_major_axis(f(%d,%d))\n"
				"\t:eccentricity(f(%d,%d))\n"
				"\t:rotation_period(f(%d,%d))\n"
				"\t:axial_tilt(fixed.deg2rad(f(%d,%d)))\n"
				"\t:rotational_phase_at_start(fixed.deg2rad(f(%d,%d)))\n"
				"\t:orbital_phase_at_start(fixed.deg2rad(f(%d,%d)))\n"
				"\t:orbital_offset(fixed.deg2rad(f(%d,%d)))\n",
				body->GetSeed(), body->GetAverageTemp(),
				int(round(body->GetOrbit().GetSemiMajorAxis() / AU * multiplier)), multiplier,
				int(round(body->GetOrbit().GetEccentricity() * multiplier)), multiplier,
				int(round(body->m_rotationPeriod.ToDouble() * multiplier)), multiplier,
				int(round(body->GetAxialTilt() * multiplier)), multiplier,
				int(round(body->m_rotationalPhaseAtStart.ToDouble() * multiplier * 180 / M_PI)), multiplier,
				int(round(body->m_orbitalPhaseAtStart.ToDouble() * multiplier * 180 / M_PI)), multiplier,
				int(round(body->m_orbitalOffset.ToDouble() * multiplier * 180 / M_PI)), multiplier);

		if (body->GetType() == SystemBody::TYPE_PLANET_TERRESTRIAL)
			fprintf(f,
				"\t:metallicity(f(%d,%d))\n"
				"\t:volcanicity(f(%d,%d))\n"
				"\t:atmos_density(f(%d,%d))\n"
				"\t:atmos_oxidizing(f(%d,%d))\n"
				"\t:ocean_cover(f(%d,%d))\n"
				"\t:ice_cover(f(%d,%d))\n"
				"\t:life(f(%d,%d))\n",
				int(round(body->GetMetallicity() * multiplier)), multiplier,
				int(round(body->GetVolcanicity() * multiplier)), multiplier,
				int(round(body->GetVolatileGas() * multiplier)), multiplier,
				int(round(body->GetAtmosOxidizing() * multiplier)), multiplier,
				int(round(body->GetVolatileLiquid() * multiplier)), multiplier,
				int(round(body->GetVolatileIces() * multiplier)), multiplier,
				int(round(body->GetLife() * multiplier)), multiplier);
	}

	fprintf(f, "\n");

	std::string code_list = code_name;
	if (body->m_children.size() > 0) {
		code_list = code_list + ", \n\t{\n";
		for (Uint32 ii = 0; ii < body->m_children.size(); ii++) {
			code_list = code_list + "\t" + ExportBodyToLua(f, body->m_children[ii]) + ", \n";
		}
		code_list = code_list + "\t}";
	}

	return code_list;
}

std::string StarSystem::GetStarTypes(SystemBody *body)
{
	int bodyTypeIdx = 0;
	std::string types = "";

	if (body->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		for (bodyTypeIdx = 0; ENUM_BodyType[bodyTypeIdx].name != 0; bodyTypeIdx++) {
			if (ENUM_BodyType[bodyTypeIdx].value == int(body->GetType()))
				break;
		}

		types = types + "'" + ENUM_BodyType[bodyTypeIdx].name + "', ";
	}

	for (Uint32 ii = 0; ii < body->m_children.size(); ii++) {
		types = types + GetStarTypes(body->m_children[ii]);
	}

	return types;
}

void StarSystem::ExportToLua(const char *filename)
{
	FILE *f = fopen(filename, "w");
	int j;

	if (f == 0)
		return;

	fprintf(f, "-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details\n");
	fprintf(f, "-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt\n\n");

	std::string stars_in_system = GetStarTypes(m_rootBody.Get());

	for (j = 0; ENUM_PolitGovType[j].name != 0; j++) {
		if (ENUM_PolitGovType[j].value == GetSysPolit().govType)
			break;
	}

	fprintf(f, "local system = CustomSystem:new('%s', { %s })\n\t:govtype('%s')\n\t:short_desc('%s')\n\t:long_desc([[%s]])\n\n",
		GetName().c_str(), stars_in_system.c_str(), ENUM_PolitGovType[j].name, GetShortDescription().c_str(), GetLongDescription().c_str());

	fprintf(f, "system:bodies(%s)\n\n", ExportBodyToLua(f, m_rootBody.Get()).c_str());

	RefCountedPtr<const Sector> sec = m_galaxy->GetSector(GetPath());
	SystemPath pa = GetPath();

	fprintf(f, "system:add_to_sector(%d,%d,%d,v(%.4f,%.4f,%.4f))\n",
		pa.sectorX, pa.sectorY, pa.sectorZ,
		sec->m_systems[pa.systemIndex].GetPosition().x / Sector::SIZE,
		sec->m_systems[pa.systemIndex].GetPosition().y / Sector::SIZE,
		sec->m_systems[pa.systemIndex].GetPosition().z / Sector::SIZE);

	fclose(f);
}

void StarSystem::Dump(FILE *file, const char *indent, bool suppressSectorData) const
{
	if (suppressSectorData) {
		fprintf(file, "%sStarSystem {%s\n", indent, m_hasCustomBodies ? " CUSTOM-ONLY" : m_isCustom ? " CUSTOM" : "");
	} else {
		fprintf(file, "%sStarSystem(%d,%d,%d,%u) {\n", indent, m_path.sectorX, m_path.sectorY, m_path.sectorZ, m_path.systemIndex);
		fprintf(file, "%s\t\"%s\"\n", indent, m_name.c_str());
		fprintf(file, "%s\t%sEXPLORED%s\n", indent, GetUnexplored() ? "UN" : "", m_hasCustomBodies ? ", CUSTOM-ONLY" : m_isCustom ? ", CUSTOM" : "");
		fprintf(file, "%s\tfaction %s%s%s\n", indent, m_faction ? "\"" : "NONE", m_faction ? m_faction->name.c_str() : "", m_faction ? "\"" : "");
		fprintf(file, "%s\tseed %u\n", indent, static_cast<Uint32>(m_seed));
		fprintf(file, "%s\t%u stars%s\n", indent, m_numStars, m_numStars > 0 ? " {" : "");
		assert(m_numStars == m_stars.size());
		for (unsigned i = 0; i < m_numStars; ++i)
			fprintf(file, "%s\t\t%s\n", indent, EnumStrings::GetString("BodyType", m_stars[i]->GetType()));
		if (m_numStars > 0) fprintf(file, "%s\t}\n", indent);
	}
	fprintf(file, "%s\t" SIZET_FMT " bodies, " SIZET_FMT " spaceports \n", indent, m_bodies.size(), m_spaceStations.size());
	fprintf(file, "%s\tpopulation %.0f\n", indent, m_totalPop.ToDouble() * 1e9);
	fprintf(file, "%s\tgovernment %s/%s, lawlessness %.2f\n", indent, m_polit.GetGovernmentDesc(), m_polit.GetEconomicDesc(),
		m_polit.lawlessness.ToDouble() * 100.0);
	const char *econ_name = !m_econType ? "NONE" : GalacticEconomy::GetEconomyById(m_econType).name;
	fprintf(file, "%s\teconomy type %s\n", indent, econ_name);
	fprintf(file, "%s\thumanProx %.2f\n", indent, m_humanProx.ToDouble() * 100.0);
	fprintf(file, "%s\tmetallicity %.2f, industrial %.2f, agricultural %.2f\n", indent, m_metallicity.ToDouble() * 100.0,
		m_industrial.ToDouble() * 100.0, m_agricultural.ToDouble() * 100.0);
	fprintf(file, "%s\ttrade levels {\n", indent);
	for (const auto &commodity : GalacticEconomy::Commodities()) {
		fprintf(file, "%s\t\t%s = %d\n", indent, commodity.name, m_tradeLevel[commodity.id - 1]);
	}
	fprintf(file, "%s\t}\n", indent);
	if (m_rootBody) {
		char buf[32];
		snprintf(buf, sizeof(buf), "%s\t", indent);
		assert(m_rootBody->GetPath().IsSameSystem(m_path));
		m_rootBody->Dump(file, buf);
	}
	fprintf(file, "%s}\n", indent);
}

void StarSystem::DumpToJson(Json &obj)
{
	obj["name"] = m_name;

	if (!m_other_names.empty())  {
		Json &out_names = obj["otherNames"] = Json::array();
		for (auto &name : m_other_names)
			out_names.push_back(name);
	}

	Json &out_types = obj["stars"] = Json::array();
	for (size_t idx = 0; idx < m_numStars; idx++)
		out_types.push_back(EnumStrings::GetString("BodyType", m_stars[idx]->GetType()));

	obj["sector"] = Json::array({ m_path.sectorX, m_path.sectorY, m_path.sectorZ });
	obj["pos"] = Json::array({ m_pos.x, m_pos.y, m_pos.z });

	obj["seed"] = m_seed;
	obj["explored"] = m_explored == ExplorationState::eEXPLORED_AT_START;

	obj["lawlessness"] = m_polit.lawlessness;

	if (m_polit.govType != Polit::GOV_INVALID)
		obj["govType"] = EnumStrings::GetString("PolitGovType", m_polit.govType);

	obj["shortDesc"] = m_shortDesc;
	obj["longDesc"] = m_longDesc;

	if (m_faction)
		obj["faction"] = m_faction->name;

	Json &bodies = obj["bodies"] = Json::array();

	for (RefCountedPtr<SystemBody> &body : m_bodies) {

		Json bodyObj = Json::object();

		body->SaveToJson(bodyObj);

		// bodyIndex is (at least informally) guaranteed to be the index in m_bodies
		if (body->GetParent())
			bodyObj["parent"] = body->GetParent()->GetPath().bodyIndex;

		if (body->HasChildren()) {
			Json &children = bodyObj["children"] = Json::array();

			for (SystemBody *child : body->GetChildren())
				children.push_back(child->GetPath().bodyIndex);
		}

		bodies.emplace_back(std::move(bodyObj));

	}
}
