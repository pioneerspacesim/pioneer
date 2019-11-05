// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StarSystem.h"

#include "Faction.h"
#include "Galaxy.h"
#include "Json.h"
#include "Sector.h"
#include "StarSystemWriter.h"

#include "EnumStrings.h"
#include "GameSaveError.h"
#include "LuaEvent.h"
#include "Orbit.h"
#include "enum_table.h"
#include <SDL_stdinc.h>
#include <algorithm>
#include <map>
#include <string>

//#define DEBUG_DUMP

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
	m_econType(GalacticEconomy::ECON_MINING),
	m_seed(0),
	m_commodityLegal(unsigned(GalacticEconomy::Commodity::COMMODITY_COUNT), true),
	m_cache(cache)
{
	PROFILE_SCOPED()
	memset(m_tradeLevel, 0, sizeof(m_tradeLevel));
}

StarSystem::~StarSystem()
{
	PROFILE_SCOPED()
	// clear parent and children pointers. someone (Lua) might still have a
	// reference to things that are about to be deleted
	m_rootBody->ClearParentAndChildPointers();
	if (m_cache)
		m_cache->RemoveFromAttic(m_path);
}

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

void StarSystem::ExploreSystem(double time)
{
	if (m_explored != eUNEXPLORED)
		return;
	m_explored = eEXPLORED_BY_PLAYER;
	m_exploredTime = time;
	RefCountedPtr<Sector> sec = m_galaxy->GetMutableSector(m_path);
	Sector::System &secsys = sec->m_systems[m_path.systemIndex];
	secsys.SetExplored(m_explored, m_exploredTime);
	StarSystemWriter a(this);
	a.MakeShortDescription();
	LuaEvent::Queue("onSystemExplored", this);
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

namespace {
	bool InvalidSystemNameChar(char c)
	{
		return !(
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9'));
	}
} // namespace

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
		if (ENUM_BodyType[bodyTypeIdx].value == body->GetType()) {
			pBodyTypeName = ENUM_BodyType[bodyTypeIdx].name;
			break;
		}
	}

	if (body->GetType() == GalaxyEnums::BodyType::TYPE_STARPORT_SURFACE) {
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

		if (body->GetType() != GalaxyEnums::BodyType::TYPE_GRAVPOINT)
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

		if (body->GetType() == GalaxyEnums::BodyType::TYPE_PLANET_TERRESTRIAL)
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

	if (body->GetSuperType() == GalaxyEnums::BodySuperType::SUPERTYPE_STAR) {
		for (bodyTypeIdx = 0; ENUM_BodyType[bodyTypeIdx].name != 0; bodyTypeIdx++) {
			if (ENUM_BodyType[bodyTypeIdx].value == body->GetType())
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

	fprintf(f, "-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details\n");
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
	fprintf(file, "%s\teconomy type%s%s%s\n", indent, m_econType == 0 ? " NONE" : m_econType & GalacticEconomy::ECON_AGRICULTURE ? " AGRICULTURE" : "",
		m_econType & GalacticEconomy::ECON_INDUSTRY ? " INDUSTRY" : "", m_econType & GalacticEconomy::ECON_MINING ? " MINING" : "");
	fprintf(file, "%s\thumanProx %.2f\n", indent, m_humanProx.ToDouble() * 100.0);
	fprintf(file, "%s\tmetallicity %.2f, industrial %.2f, agricultural %.2f\n", indent, m_metallicity.ToDouble() * 100.0,
		m_industrial.ToDouble() * 100.0, m_agricultural.ToDouble() * 100.0);
	fprintf(file, "%s\ttrade levels {\n", indent);
	for (int i = 1; i < GalacticEconomy::COMMODITY_COUNT; ++i) {
		fprintf(file, "%s\t\t%s = %d\n", indent, EnumStrings::GetString("CommodityType", i), m_tradeLevel[i]);
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
