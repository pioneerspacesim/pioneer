// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StarSystemGenerator.h"

#include "AtmosphereParameters.h"
#include "Factions.h"
#include "Galaxy.h"
#include "Json.h"
#include "Lang.h"
#include "Pi.h"
#include "Sector.h"
#include "gameconsts.h"
#include "core/Log.h"
#include "core/macros.h"
#include "galaxy/Economy.h"
#include "lua/LuaNameGen.h"
#include "profiler/Profiler.h"

#include <functional>

static const fixed SUN_MASS_TO_EARTH_MASS = fixed(332998, 1); // XXX Duplication from StarSystem.cpp
// if binary stars have separation s, planets can have stable
// orbits at (0.5 * s * SAFE_DIST_FROM_BINARY)
static const fixed SAFE_DIST_FROM_BINARY = fixed(5, 1);
// very crudely
static const fixed AU_SOL_RADIUS = fixed(305, 65536);
static const fixed AU_EARTH_RADIUS = fixed(3, 65536); // XXX Duplication from StarSystem.cpp
static const fixed FIXED_PI = fixed(103993, 33102);	  // XXX Duplication from StarSystem.cpp
static const double CELSIUS = 273.15;

// max surface gravity for a permanent human settlement
static const double MAX_SETTLEMENT_SURFACE_GRAVITY = 50; // m/s2 .. roughly 5 g

static const Uint32 POLIT_SEED = 0x1234abcd;
static const Uint32 POLIT_SALT = 0x8732abdf;

const fixed StarSystemLegacyGeneratorBase::starMetallicities[] = {
	fixed(1, 1),	// GRAVPOINT - for planets that orbit them
	fixed(9, 10),	// brown dwarf
	fixed(5, 10),	// white dwarf
	fixed(7, 10),	// M0
	fixed(6, 10),	// K0
	fixed(5, 10),	// G0
	fixed(4, 10),	// F0
	fixed(3, 10),	// A0
	fixed(2, 10),	// B0
	fixed(1, 10),	// O5
	fixed(8, 10),	// M0 Giant
	fixed(65, 100), // K0 Giant
	fixed(55, 100), // G0 Giant
	fixed(4, 10),	// F0 Giant
	fixed(3, 10),	// A0 Giant
	fixed(2, 10),	// B0 Giant
	fixed(1, 10),	// O5 Giant
	fixed(9, 10),	// M0 Super Giant
	fixed(7, 10),	// K0 Super Giant
	fixed(6, 10),	// G0 Super Giant
	fixed(4, 10),	// F0 Super Giant
	fixed(3, 10),	// A0 Super Giant
	fixed(2, 10),	// B0 Super Giant
	fixed(1, 10),	// O5 Super Giant
	fixed(1, 1),	// M0 Hyper Giant
	fixed(7, 10),	// K0 Hyper Giant
	fixed(6, 10),	// G0 Hyper Giant
	fixed(4, 10),	// F0 Hyper Giant
	fixed(3, 10),	// A0 Hyper Giant
	fixed(2, 10),	// B0 Hyper Giant
	fixed(1, 10),	// O5 Hyper Giant
	fixed(1, 1),	// M WF
	fixed(8, 10),	// B WF
	fixed(6, 10),	// O WF
	fixed(1, 1),	//  S BH	Blackholes, give them high metallicity,
	fixed(1, 1),	// IM BH	so any rocks that happen to be there
	fixed(1, 1)		// SM BH	may be mining hotspots. FUN :)
};

const StarSystemLegacyGeneratorBase::StarTypeInfo StarSystemLegacyGeneratorBase::starTypeInfo[] = {
	{ {}, {},
		0, 0 },
	{ //Brown Dwarf
		{ 2, 8 }, { 10, 30 },
		1000, 2000 },
	{ //white dwarf
		{ 20, 100 }, { 1, 2 },
		4000, 40000 },
	{ //M
		{ 10, 47 }, { 30, 60 },
		2000, 3500 },
	{ //K
		{ 50, 78 }, { 60, 100 },
		3500, 5000 },
	{ //G
		{ 80, 110 }, { 80, 120 },
		5000, 6000 },
	{ //F
		{ 115, 170 }, { 110, 150 },
		6000, 7500 },
	{ //A
		{ 180, 320 }, { 120, 220 },
		7500, 10000 },
	{ //B
		{ 200, 300 }, { 120, 290 },
		10000, 30000 },
	{ //O
		{ 300, 400 }, { 200, 310 },
		30000, 60000 },
	{ //M Giant
		{ 60, 357 }, { 2000, 5000 },
		2500, 3500 },
	{ //K Giant
		{ 125, 500 }, { 1500, 3000 },
		3500, 5000 },
	{ //G Giant
		{ 200, 800 }, { 1000, 2000 },
		5000, 6000 },
	{ //F Giant
		{ 250, 900 }, { 800, 1500 },
		6000, 7500 },
	{ //A Giant
		{ 400, 1000 }, { 600, 1000 },
		7500, 10000 },
	{ //B Giant
		{ 500, 1000 }, { 600, 1000 },
		10000, 30000 },
	{ //O Giant
		{ 600, 1200 }, { 600, 1000 },
		30000, 60000 },
	{ //M Super Giant
		{ 1050, 5000 }, { 7000, 15000 },
		2500, 3500 },
	{ //K Super Giant
		{ 1100, 5000 }, { 5000, 9000 },
		3500, 5000 },
	{ //G Super Giant
		{ 1200, 5000 }, { 4000, 8000 },
		5000, 6000 },
	{ //F Super Giant
		{ 1500, 6000 }, { 3500, 7000 },
		6000, 7500 },
	{ //A Super Giant
		{ 2000, 8000 }, { 3000, 6000 },
		7500, 10000 },
	{ //B Super Giant
		{ 3000, 9000 }, { 2500, 5000 },
		10000, 30000 },
	{ //O Super Giant
		{ 5000, 10000 }, { 2000, 4000 },
		30000, 60000 },
	{ //M Hyper Giant
		{ 5000, 15000 }, { 20000, 40000 },
		2500, 3500 },
	{ //K Hyper Giant
		{ 5000, 17000 }, { 17000, 25000 },
		3500, 5000 },
	{ //G Hyper Giant
		{ 5000, 18000 }, { 14000, 20000 },
		5000, 6000 },
	{ //F Hyper Giant
		{ 5000, 19000 }, { 12000, 17500 },
		6000, 7500 },
	{ //A Hyper Giant
		{ 5000, 20000 }, { 10000, 15000 },
		7500, 10000 },
	{ //B Hyper Giant
		{ 5000, 23000 }, { 6000, 10000 },
		10000, 30000 },
	{ //O Hyper Giant
		{ 10000, 30000 }, { 4000, 7000 },
		30000, 60000 },
	{ // M WF
		{ 2000, 5000 }, { 2500, 5000 },
		25000, 35000 },
	{ // B WF
		{ 2000, 7500 }, { 2500, 5000 },
		35000, 45000 },
	{ // O WF
		{ 2000, 10000 }, { 2500, 5000 },
		45000, 60000 },
	{							// S BH
		{ 20, 2000 }, { 0, 0 }, // XXX black holes are < 1 Sol radii big; this is clamped to a non-zero value later
		10, 24 },
	{ // IM BH
		{ 900000, 1000000 }, { 100, 500 },
		1, 10 },
	{ // SM BH
		{ 2000000, 5000000 }, { 10000, 20000 },
		10, 24 }
};

bool StarSystemFromSectorGenerator::Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config)
{
	PROFILE_SCOPED()
	RefCountedPtr<const Sector> sec = galaxy->GetSector(system->GetPath());
	assert(system->GetPath().systemIndex < sec->m_systems.size());
	const Sector::System &secSys = sec->m_systems[system->GetPath().systemIndex];

	system->SetFaction(galaxy->GetFactions()->GetNearestClaimant(&secSys));
	system->SetSeed(secSys.GetSeed());
	system->SetName(secSys.GetName());
	system->SetOtherNames(secSys.GetOtherNames());
	system->SetExplored(secSys.GetExplored(), secSys.GetExploredTime());
	return true;
}

void StarSystemLegacyGeneratorBase::PickAtmosphere(SystemBody *sbody)
{
	/* Alpha value isn't real alpha. in the shader fog depth is determined
	 * by density*alpha, so that we can have very dense atmospheres
	 * without having them a big stinking solid color obscuring everything

	  These are our atmosphere colours, for terrestrial planets we use m_atmosOxidizing
	  for some variation to atmosphere colours
	 */
	switch (sbody->GetType()) {
	case SystemBody::TYPE_PLANET_GAS_GIANT:

		sbody->m_atmosColor = Color(255, 255, 255, 3);
		// NOTE: realistic generation for gas giant atmospheres needed elsewhere
		// sbody->m_atmosDensity = 14.0;
		break;
	case SystemBody::TYPE_PLANET_ASTEROID:
		sbody->m_atmosColor = Color::BLANK;
		break;
	default:
	case SystemBody::TYPE_PLANET_TERRESTRIAL:
		double r = 0, g = 0, b = 0;
		double atmo = sbody->GetAtmosOxidizing();
		if (sbody->GetVolatileGas() > 0.001) {
			if (atmo > 0.95) {
				// o2
				r = 1.0f + ((0.95f - atmo) * 15.0f);
				g = 0.95f + ((0.95f - atmo) * 10.0f);
				b = atmo * atmo * atmo * atmo * atmo;
			} else if (atmo > 0.7) {
				// co2
				r = atmo + 0.05f;
				g = 1.0f + (0.7f - atmo);
				b = 0.8f;
			} else if (atmo > 0.65) {
				// co
				r = 1.0f + (0.65f - atmo);
				g = 0.8f;
				b = atmo + 0.25f;
			} else if (atmo > 0.55) {
				// ch4
				r = 1.0f + ((0.55f - atmo) * 5.0);
				g = 0.35f - ((0.55f - atmo) * 5.0);
				b = 0.4f;
			} else if (atmo > 0.3) {
				// h
				r = 1.0f;
				g = 1.0f;
				b = 1.0f;
			} else if (atmo > 0.2) {
				// he
				r = 1.0f;
				g = 1.0f;
				b = 1.0f;
			} else if (atmo > 0.15) {
				// ar
				r = 0.5f - ((0.15f - atmo) * 5.0);
				g = 0.0f;
				b = 0.5f + ((0.15f - atmo) * 5.0);
			} else if (atmo > 0.1) {
				// s
				r = 0.8f - ((0.1f - atmo) * 4.0);
				g = 1.0f;
				b = 0.5f - ((0.1f - atmo) * 10.0);
			} else {
				// n
				r = 1.0f;
				g = 1.0f;
				b = 1.0f;
			}
			sbody->m_atmosColor = Color(r * 255, g * 255, b * 255, 255);
		} else {
			sbody->m_atmosColor = Color::BLANK;
		}
		//Output("| Atmosphere :\n|      red   : [%f] \n|      green : [%f] \n|      blue  : [%f] \n", r, g, b);
		//Output("-------------------------------\n");
		break;
		/*default:
			sbody->m_atmosColor = Color(0.6f, 0.6f, 0.6f, 1.0f);
			sbody->m_atmosDensity = m_body->m_volatileGas.ToDouble();
			break;*/
	}
}

static const unsigned char RANDOM_RING_COLORS[][4] = {
	{ 156, 122, 98, 217 },	// jupiter-like
	{ 156, 122, 98, 217 },	// saturn-like
	{ 181, 173, 174, 217 }, // neptune-like
	{ 130, 122, 98, 217 },	// uranus-like
	{ 207, 122, 98, 217 }	// brown dwarf-like
};

void StarSystemLegacyGeneratorBase::PickRings(SystemBodyData *sbody, bool forceRings)
{
	sbody->m_rings.minRadius = fixed();
	sbody->m_rings.maxRadius = fixed();
	sbody->m_rings.baseColor = Color(255, 255, 255, 255);

	bool bHasRings = forceRings;
	if (!bHasRings) {
		Random ringRng(sbody->m_seed + 965467);
		// today's forecast:
		if (sbody->m_type == SystemBody::TYPE_PLANET_GAS_GIANT) {
			// 50% chance of rings
			bHasRings = ringRng.Double() < 0.5;
		} else if (sbody->m_type == SystemBody::TYPE_PLANET_TERRESTRIAL) {
			// 10% chance of rings
			bHasRings = ringRng.Double() < 0.1;
		}
		/*else if (sbody->m_type == SystemBody::TYPE_PLANET_ASTEROID)
		{
			// 1:10 (10%) chance of rings
			bHasRings = ringRng.Double() < 0.1;
		}*/
	}

	if (bHasRings) {
		Random ringRng(sbody->m_seed + 965467);

		// today's forecast: 50% chance of rings
		double rings_die = ringRng.Double();
		if (forceRings || (rings_die < 0.5)) {
			const unsigned char *const baseCol = RANDOM_RING_COLORS[ringRng.Int32(COUNTOF(RANDOM_RING_COLORS))];
			sbody->m_rings.baseColor.r = Clamp(baseCol[0] + ringRng.Int32(-20, 20), 0, 255);
			sbody->m_rings.baseColor.g = Clamp(baseCol[1] + ringRng.Int32(-20, 20), 0, 255);
			sbody->m_rings.baseColor.b = Clamp(baseCol[2] + ringRng.Int32(-20, 10), 0, 255);
			sbody->m_rings.baseColor.a = Clamp(baseCol[3] + ringRng.Int32(-5, 5), 0, 255);

			// from wikipedia: http://en.wikipedia.org/wiki/Roche_limit
			// basic Roche limit calculation assuming a rigid satellite
			// d = R (2 p_M / p_m)^{1/3}
			//
			// where R is the radius of the primary, p_M is the density of
			// the primary and p_m is the density of the satellite
			//
			// I assume a satellite density of 500 kg/m^3
			// (which Wikipedia says is an average comet density)
			//
			// also, I can't be bothered to think about unit conversions right now,
			// so I'm going to ignore the real density of the primary and take it as 1100 kg/m^3
			// (note: density of Saturn is ~687, Jupiter ~1,326, Neptune ~1,638, Uranus ~1,318)
			//
			// This gives: d = 1.638642 * R
			fixed innerMin = fixed(110, 100);
			fixed innerMax = fixed(145, 100);
			fixed outerMin = fixed(150, 100);
			fixed outerMax = fixed(168642, 100000);

			sbody->m_rings.minRadius = innerMin + (innerMax - innerMin) * ringRng.Fixed();
			sbody->m_rings.maxRadius = outerMin + (outerMax - outerMin) * ringRng.Fixed();
		}
	}
}

/*
 * http://en.wikipedia.org/wiki/Hill_sphere
 */
fixedf<48> StarSystemLegacyGeneratorBase::CalcHillRadius(SystemBody *sbody) const
{
	PROFILE_SCOPED()
	// high-precision for working with very small numbers
	// system distances are not expected to be larger than 32k AU
	using fixedp = fixedf<48>;

	if (sbody->GetSuperType() <= SystemBody::SUPERTYPE_STAR) {
		return fixedp();
	} else {
		// playing with precision since these numbers get small
		// masses in earth masses
		fixed mprimary = sbody->GetParent()->GetMassInEarths();

		fixedp a = sbody->GetSemiMajorAxisAsFixed();
		fixedp e = sbody->GetEccentricityAsFixed();
		fixedp pe = a * (fixedp(1, 1) - e); // periapsis in higher precision

		return pe * fixedp::CubeRootOf(sbody->GetMassAsFixed() / (fixed(3, 1) * mprimary));

		//fixed hr = semiMajorAxis*(fixed(1,1) - eccentricity) *
		//  fixedcuberoot(mass / (3*mprimary));
	}
}

void StarSystemCustomGenerator::CustomGetKidsOf(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *parent,
	const std::vector<CustomSystemBody *> &children, int *outHumanInfestedness)
{
	PROFILE_SCOPED()

	// gravpoints have no mass, but we sum the masses of its children instead
	if (parent->GetType() == SystemBody::TYPE_GRAVPOINT) {
		parent->m_mass = fixed(0);

		// parent gravpoint mass = sum of masses of its children
		for (const auto *child : children) {
			if (child->bodyData.m_type > SystemBody::TYPE_GRAVPOINT && child->bodyData.m_type <= SystemBody::TYPE_STAR_MAX)
				parent->m_mass += child->bodyData.m_mass;
			else
				parent->m_mass += child->bodyData.m_mass / SUN_MASS_TO_EARTH_MASS;
		}
	}

	for (std::vector<CustomSystemBody *>::const_iterator i = children.begin(); i != children.end(); ++i) {
		const CustomSystemBody *csbody = *i;

		SystemBody *kid = system->NewBody();
		kid->m_parent = parent;
		kid->m_isCustomBody = true;

		// Copy all system body parameters from the custom system body
		*kid = csbody->bodyData;

		kid->SetOrbitFromParameters();
		kid->SetAtmFromParameters();

		if (kid->GetType() != SystemBody::TYPE_STARPORT_SURFACE) {
			if (kid->GetSuperType() == SystemBody::SUPERTYPE_STARPORT) {
				fixed lowestOrbit = fixed().FromDouble(parent->CalcAtmosphereParams().atmosRadius + 500000.0 / EARTH_RADIUS);
				if (kid->GetOrbit().GetSemiMajorAxis() < lowestOrbit.ToDouble()) {
					Error("%s's orbit is too close to its parent (%.2f/%.2f)", kid->m_name.c_str(), kid->GetOrbit().GetSemiMajorAxis(), lowestOrbit.ToFloat());
				}
			} else {
				if (kid->GetOrbit().GetSemiMajorAxis() < 1.2 * parent->GetRadius()) {
					Error("%s's orbit is too close to its parent", kid->m_name.c_str());
				}
			}
		}

		if (kid->GetSuperType() == SystemBody::SUPERTYPE_STARPORT) {
			(*outHumanInfestedness)++;
			system->AddSpaceStation(kid);
		}
		parent->m_children.push_back(kid);

		PickAtmosphere(kid);

		CustomGetKidsOf(system, kid, csbody->children, outHumanInfestedness);
	}
}

bool StarSystemCustomGenerator::ApplyToSystem(Random &rng, RefCountedPtr<StarSystem::GeneratorAPI> system, const CustomSystem *customSys)
{
	system->SetCustom(true, false);
	system->SetNumStars(customSys->numStars);
	system->SetPosition(customSys->pos);
	system->SetOtherNames(customSys->other_names);

	if (customSys->name.length() > 0) system->SetName(customSys->name);
	if (customSys->shortDesc.length() > 0) system->SetShortDesc(customSys->shortDesc);
	if (customSys->longDesc.length() > 0) system->SetLongDesc(customSys->longDesc);

	SysPolit sysPolit;
	sysPolit.govType = customSys->govType;
	sysPolit.lawlessness = customSys->lawlessness;

	system->SetSysPolit(sysPolit);

	if (customSys->IsRandom())
		return false;

	system->SetCustom(true, true);

	const CustomSystemBody *csbody = customSys->sBody;

	SystemBody *rootBody = system->NewBody();
	*rootBody = csbody->bodyData;
	rootBody->m_parent = 0;
	rootBody->m_isCustomBody = true;

	system->SetRootBody(rootBody);

	int humanInfestedness = 0;
	CustomGetKidsOf(system, rootBody, csbody->children, &humanInfestedness);
	unsigned countedStars = 0;
	for (RefCountedPtr<SystemBody> b : system->GetBodies()) {
		if (b->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
			++countedStars;
			system->AddStar(b.Get());
		}
	}

	(void) countedStars;
	assert(countedStars == system->GetNumStars());
	return true;
}

bool StarSystemCustomGenerator::Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config)
{
	PROFILE_SCOPED()
	RefCountedPtr<const Sector> sec = galaxy->GetSector(system->GetPath());
	system->SetCustom(false, false);

	if (const CustomSystem *customSys = sec->m_systems[system->GetPath().systemIndex].GetCustomSystem())
		config->isCustomOnly = ApplyToSystem(rng, system, customSys);

	return true;
}

/*
 * star_radius in sol radii
 * star_temp in kelvin,
 * object_dist in AU
 * return energy per unit area in solar constants (1362 W/m^2 )
 */
static fixed calcEnergyPerUnitAreaAtDist(fixed star_radius, int star_temp, fixed object_dist)
{
	// energy = boltzmann * T^4 * 4 * PI * r^2
	// energy_per_m2 = energy / ( 4 * PI * dist^2 )
	// drop 4*PI because it directly cancels
	// energy_per_m2 is later divided by the boltzmann constant so drop that too
	fixed temp = star_temp * fixed(1, 5778); //normalize to Sun's temperature
	const fixed total_solar_emission =
		temp * temp * temp * temp * star_radius * star_radius;

	return total_solar_emission / (object_dist * object_dist); //return value in solar consts (overflow prevention)
}

//helper function, get branch of system tree from body all the way to the system's root and write it to path
static void getPathToRoot(const SystemBody *body, std::vector<const SystemBody *> &path)
{
	while (body) {
		path.push_back(body);
		body = body->GetParent();
	}
}

int StarSystemRandomGenerator::CalcSurfaceTemp(const SystemBody *primary, fixed distToPrimary, fixed albedo, fixed greenhouse)
{
	PROFILE_SCOPED()

	// accumulator seeded with current primary
	fixed energy_per_meter2 = calcEnergyPerUnitAreaAtDist(primary->m_radius, primary->m_averageTemp, distToPrimary);
	fixed dist;
	// find the other stars which aren't our parent star
	for (auto *s : primary->GetStarSystem()->GetStars()) {
		if (s != primary) {
			//get branches from body and star to system root
			std::vector<const SystemBody *> first_to_root;
			std::vector<const SystemBody *> second_to_root;
			getPathToRoot(primary, first_to_root);
			getPathToRoot(&(*s), second_to_root);
			std::vector<const SystemBody *>::reverse_iterator fit = first_to_root.rbegin();
			std::vector<const SystemBody *>::reverse_iterator sit = second_to_root.rbegin();
			// keep tracing both branches from system's root until they diverge
			while (sit != second_to_root.rend() && fit != first_to_root.rend() && (*sit) == (*fit)) {
				++sit;
				++fit;
			}
			if (sit == second_to_root.rend()) --sit;
			if (fit == first_to_root.rend()) --fit; //oops! one of the branches ends at lca, backtrack

			//planet is around one part of coorbiting pair, star is another.
			if ((*fit)->IsCoOrbitalWith(*sit)) {
				dist = ((*fit)->GetOrbMaxAsFixed() + (*fit)->GetOrbMinAsFixed()) >> 1; //binaries don't have fully initialized smaxes
			} else if ((*sit)->IsCoOrbital()) {
				//star is part of binary around which planet is (possibly indirectly) orbiting
				bool inverted_ancestry = false;
				for (const SystemBody *body = (*sit); body; body = body->GetParent())
					if (body == (*fit)) {
						inverted_ancestry = true; //ugly hack due to function being static taking planet's primary rather than being called from actual planet
						break;
					}
				if (inverted_ancestry) //primary is star's ancestor! Don't try to take its orbit (could probably be a gravpoint check at this point, but paranoia)
				{
					dist = distToPrimary;
				} else {
					dist = ((*fit)->GetOrbMaxAsFixed() + (*fit)->GetOrbMinAsFixed()) >> 1; //simplified to planet orbiting stationary star
				}
			} else if ((*fit)->IsCoOrbital()) //planet is around one part of coorbiting pair, star isn't coorbiting with it
			{
				//simplified to star orbiting stationary planet. neither is part of any binaries - hooray!
				dist = ((*sit)->GetOrbMaxAsFixed() + (*sit)->GetOrbMinAsFixed()) >> 1;
			} else {
				//avg of conjunction and opposition dist
				dist = (((*sit)->GetSemiMajorAxisAsFixed() - (*fit)->GetSemiMajorAxisAsFixed()).Abs() + ((*sit)->GetSemiMajorAxisAsFixed() + (*fit)->GetSemiMajorAxisAsFixed()));
				dist >>= 1;
			}
		}
		energy_per_meter2 += calcEnergyPerUnitAreaAtDist(s->m_radius, s->m_averageTemp, dist);
	}

	/*
	// Can't use this version as pow() is nowhere near deterministic across multiple platforms and compilers
	// Luckily pow(x, 0.25) can be expressed as two successive sqrt operations
	// bond albedo, not geometric
	static double CalcSurfaceTemp(double star_radius, double star_temp, double object_dist, double albedo, double greenhouse)
	{
		const double energy_per_meter2 = calcEnergyPerUnitAreaAtDist(star_radius, star_temp, object_dist);
		const double surface_temp = pow(energy_per_meter2*(1-albedo)/(4*(1-greenhouse)*boltzman_const), 0.25);
		return surface_temp;
	}
	*/

	const fixed surface_temp_pow4 = energy_per_meter2 * (1 - albedo) / (1 - greenhouse);
	return (279 * int(isqrt(isqrt((surface_temp_pow4.v))))) >> (fixed::FRAC / 4); //multiplied by 279 to convert from Earth's temps to Kelvin
}

/*
 * For moons distance from star is not orbMin, orbMax.
 */
const SystemBody *StarSystemRandomGenerator::FindStarAndTrueOrbitalRange(const SystemBody *planet, fixed &orbMin_, fixed &orbMax_) const
{
	const SystemBody *star = planet->GetParent();

	assert(star);

	/* while not found star yet.. */
	while (star->GetSuperType() > SystemBody::SUPERTYPE_STAR) {
		planet = star;
		star = star->GetParent();
	}

	orbMin_ = planet->GetOrbMinAsFixed();
	orbMax_ = planet->GetOrbMaxAsFixed();
	return star;
}

/**
 * In this and following functions, we attempt to capture even a fraction of the
 * true majesty of the infinite cosmos.
 *
 * System generation is based primarily around a mass-based metric, where the
 * total mass of a primary body determines the maximum mass of its individual
 * satellites.
 *
 * The area of a body's "orbital slice" (between itself and any neighboring
 * bodies) informs the mass of a body to avoid obviously-unnatural
 * configurations with high-mass bodies orbiting so closely as to perturb each
 * others' orbits beyond what Pioneer can simulate.
 *
 * A series of post-processing factors are applied to this initial maximum mass
 * value to curve the mass factor over the entire Hill Sphere of the parent
 * body. This reduces the incidence of "gas giant spam" and produces more
 * perceptually-realistic arrangements of body types and masses.
 *
 * Body radius and type is inferred from the mass of the body and applied based
 * on a set of density heuristics (in PickPlanetType), and remaining body
 * parameters are set to complete body generation.
 */
void StarSystemRandomGenerator::PickPlanetType(SystemBody *sbody, Random &rand)
{
	PROFILE_SCOPED()
	fixed albedo;
	fixed greenhouse;

	fixed minDistToStar, maxDistToStar, averageDistToStar;
	const SystemBody *star = FindStarAndTrueOrbitalRange(sbody, minDistToStar, maxDistToStar);
	averageDistToStar = (minDistToStar + maxDistToStar) >> 1;

	/* first calculate blackbody temp (no greenhouse effect, zero albedo) */
	int bbody_temp = CalcSurfaceTemp(star, averageDistToStar, albedo, greenhouse);

	sbody->m_averageTemp = bbody_temp;

	static const fixed ONEEUMASS = fixed::FromDouble(1);
	static const fixed TWOHUNDREDEUMASSES = fixed::FromDouble(200.0);
	// We get some more fractional bits for small bodies otherwise we can easily end up with 0 radius which breaks stuff elsewhere
	//
	// AndyC - Updated to use the empirically gathered data from this site:
	// https://phl.upr.edu/library/labnotes/standard-mass-radius-relation-for-exoplanets
	// but we still limit at the lowest end
	if (sbody->GetMassAsFixed() <= fixed(1, 1)) {
		sbody->m_radius = fixed(fixedf<48>::CubeRootOf(fixedf<48>(sbody->GetMassAsFixed())));
	} else if (sbody->GetMassAsFixed() < ONEEUMASS) {
		// smaller than 1 Earth mass is almost certainly a rocky body
		sbody->m_radius = fixed::FromDouble(pow(sbody->GetMassAsFixed().ToDouble(), 0.3));
	} else if (sbody->GetMassAsFixed() < TWOHUNDREDEUMASSES) {
		// from 1 EU to 200 they transition from Earth-like rocky bodies, through Ocean worlds and on to Gas Giants
		sbody->m_radius = fixed::FromDouble(pow(sbody->GetMassAsFixed().ToDouble(), 0.5));
	} else {
		// Anything bigger than 200 EU masses is a Gas Giant or bigger but the density changes to decrease from here on up...
		sbody->m_radius = fixed::FromDouble(22.6 * (1.0 / pow(sbody->GetMassAsFixed().ToDouble(), double(0.0886))));
	}
	// randomize radius or the surface gravity will be consistently 1.0 g for planets in the 1 - 200 EU range.
	sbody->m_radius = fixed::FromDouble(sbody->GetRadiusAsFixed().ToDouble() * ( 1.2 - (0.4 * rand.Double()))); // +/-20% radius

	// enforce minimum size of 10km
	sbody->m_radius = std::max(sbody->GetRadiusAsFixed(), fixed(1, 630));

	if (sbody->GetParent()->GetType() <= SystemBody::TYPE_STAR_MAX) {
		// get it from the table now rather than setting it on stars/gravpoints as
		// currently nothing else needs them to have metallicity
		sbody->m_metallicity = starMetallicities[sbody->GetParent()->GetType()] * rand.Fixed();
	} else {
		// this assumes the parent's parent is a star/gravpoint, which is currently always true
		sbody->m_metallicity = starMetallicities[sbody->GetParent()->GetParent()->GetType()] * rand.Fixed();
	}

	// harder to be volcanic when you are tiny (you cool down)
	sbody->m_volcanicity = std::min(fixed(1, 1), sbody->GetMassAsFixed()) * rand.Fixed();
	sbody->m_atmosOxidizing = rand.Fixed();
	sbody->m_life = fixed();
	sbody->m_volatileGas = fixed();
	sbody->m_volatileLiquid = fixed();
	sbody->m_volatileIces = fixed();

	// pick body type
	if (sbody->GetMassAsFixed() > 317 * 13) {
		// more than 13 jupiter masses can fuse deuterium - is a brown dwarf
		sbody->m_type = SystemBody::TYPE_BROWN_DWARF;
		sbody->m_averageTemp = sbody->GetAverageTemp() + rand.Int32(starTypeInfo[sbody->GetType()].tempMin, starTypeInfo[sbody->GetType()].tempMax);
		// prevent mass exceeding 65 jupiter masses or so, when it becomes a star
		// XXX since TYPE_BROWN_DWARF is supertype star, mass is now in
		// solar masses. what a fucking mess
		sbody->m_mass = std::min(sbody->GetMassAsFixed(), fixed(317 * 65, 1)) / SUN_MASS_TO_EARTH_MASS;
		//Radius is too high as it now uses the planetary calculations to work out radius (Cube root of mass)
		// So tell it to use the star data instead:
		sbody->m_radius = fixed(rand.Int32(starTypeInfo[sbody->GetType()].radius[0], starTypeInfo[sbody->GetType()].radius[1]), 100);
	} else if (sbody->GetMassAsFixed() > 6) {
		sbody->m_type = SystemBody::TYPE_PLANET_GAS_GIANT;
		// Generate a random "surface" density for gas giants roughly fitted to real-life estimation of Jupiter at "cloud deck" level
		// The bounds are derived from real-world density-at-1-bar data for the outer gas giants with an
		// approximation factor for density at cloud deck level of `3e * density @ 1 bar`
		sbody->m_volatileGas = rand.NormFixed(fixed(1050, 1000), fixed(8000, 1000)).Abs();
		// Most gas giant atmospheres contain an incredibly small or negligible proportion of oxidizing elements / water ice
		sbody->m_atmosOxidizing = rand.NormFixed(fixed(0, 1), fixed(300, 1000)).Abs();
	} else if (sbody->GetMassAsFixed() > fixed(1, 12000)) {
		sbody->m_type = SystemBody::TYPE_PLANET_TERRESTRIAL;

		fixed amount_volatiles = fixed(2, 1) * rand.Fixed();
		if (rand.Int32(3)) amount_volatiles *= sbody->GetMassAsFixed();
		// total atmosphere loss
		if (rand.Fixed() > sbody->GetMassAsFixed()) amount_volatiles = fixed();

		//Output("Amount volatiles: %f\n", amount_volatiles.ToFloat());
		// fudge how much of the volatiles are in which state
		greenhouse = fixed();
		albedo = fixed();
		// CO2 sublimation
		if (sbody->GetAverageTemp() > 195)
			greenhouse += amount_volatiles * fixed(1, 3);
		else
			albedo += fixed(2, 6);
		// H2O liquid
		if (sbody->GetAverageTemp() > 273)
			greenhouse += amount_volatiles * fixed(1, 5);
		else
			albedo += fixed(3, 6);
		// H2O boils
		if (sbody->GetAverageTemp() > 373) greenhouse += amount_volatiles * fixed(1, 3);

		if (greenhouse > fixed(7, 10)) { // never reach 1, but 1/(1-greenhouse) still grows
			greenhouse *= greenhouse;
			greenhouse *= greenhouse;
			greenhouse = greenhouse / (greenhouse + fixed(32, 311));
		}

		sbody->m_averageTemp = CalcSurfaceTemp(star, averageDistToStar, albedo, greenhouse);

		const fixed proportion_gas = sbody->GetAverageTemp() / (fixed(100, 1) + sbody->GetAverageTemp());
		sbody->m_volatileGas = proportion_gas * amount_volatiles;

		const fixed proportion_liquid = (fixed(1, 1) - proportion_gas) * (sbody->GetAverageTemp() / (fixed(50, 1) + sbody->GetAverageTemp()));
		sbody->m_volatileLiquid = proportion_liquid * amount_volatiles;

		const fixed proportion_ices = fixed(1, 1) - (proportion_gas + proportion_liquid);
		sbody->m_volatileIces = proportion_ices * amount_volatiles;

		//Output("temp %dK, gas:liquid:ices %f:%f:%f\n", averageTemp, proportion_gas.ToFloat(),
		//		proportion_liquid.ToFloat(), proportion_ices.ToFloat());

		if ((sbody->GetVolatileLiquidAsFixed() > fixed()) &&
			(sbody->GetAverageTemp() > CELSIUS - 60) &&
			(sbody->GetAverageTemp() < CELSIUS + 200)) {
			// try for life
			int minTemp = CalcSurfaceTemp(star, maxDistToStar, albedo, greenhouse);
			int maxTemp = CalcSurfaceTemp(star, minDistToStar, albedo, greenhouse);

			if ((minTemp > CELSIUS - 10) && (minTemp < CELSIUS + 90) && //removed explicit checks for star type (also BD and WD seem to have slight chance of having life around them)
				(maxTemp > CELSIUS - 10) && (maxTemp < CELSIUS + 90))	//TODO: ceiling based on actual boiling point on the planet, not in 1atm
			{
				fixed maxMass, lifeMult, allowedMass(1, 2);
				allowedMass += 2;
				//find the most massive star, mass is tied to lifespan
				//this automagically eliminates O, B and so on from consideration
				//handy calculator: http://www.asc-csa.gc.ca/eng/educators/resources/astronomy/module2/calculator.asp
				//system could have existed long enough for life to form (based on Sol)
				for (auto *s : sbody->GetStarSystem()->GetStars()) {
					maxMass = maxMass < s->GetMassAsFixed() ? s->GetMassAsFixed() : maxMass;
				}
				if (maxMass < allowedMass) {
					lifeMult = allowedMass - maxMass;
				}
				sbody->m_life = lifeMult * rand.Fixed();
			}
		}
	} else {
		sbody->m_type = SystemBody::TYPE_PLANET_ASTEROID;
	}

	// Tidal lock for planets close to their parents:
	//		http://en.wikipedia.org/wiki/Tidal_locking
	//
	//		Formula: time ~ semiMajorAxis^6 * radius / mass / parentMass^2
	//
	//		compared to Earth's Moon
	static fixed MOON_TIDAL_LOCK = fixed(6286, 1);
	fixed invTidalLockTime = fixed(1, 1);

	// fine-tuned not to give overflows, order of evaluation matters!
	if (sbody->GetParent()->GetType() <= SystemBody::TYPE_STAR_MAX) {
		invTidalLockTime /= (sbody->GetSemiMajorAxisAsFixed() * sbody->GetSemiMajorAxisAsFixed());
		invTidalLockTime *= sbody->GetMassAsFixed();
		invTidalLockTime /= (sbody->GetSemiMajorAxisAsFixed() * sbody->GetSemiMajorAxisAsFixed());
		invTidalLockTime *= sbody->GetParent()->GetMassAsFixed() * sbody->GetParent()->GetMassAsFixed();
		invTidalLockTime /= sbody->GetRadiusAsFixed();
		invTidalLockTime /= (sbody->GetSemiMajorAxisAsFixed() * sbody->GetSemiMajorAxisAsFixed()) * MOON_TIDAL_LOCK;
	} else {
		invTidalLockTime /= (sbody->GetSemiMajorAxisAsFixed() * sbody->GetSemiMajorAxisAsFixed()) * SUN_MASS_TO_EARTH_MASS;
		invTidalLockTime *= sbody->GetMassAsFixed();
		invTidalLockTime /= (sbody->GetSemiMajorAxisAsFixed() * sbody->GetSemiMajorAxisAsFixed()) * SUN_MASS_TO_EARTH_MASS;
		invTidalLockTime *= sbody->GetParent()->GetMassAsFixed() * sbody->GetParent()->GetMassAsFixed();
		invTidalLockTime /= sbody->GetRadiusAsFixed();
		invTidalLockTime /= (sbody->GetSemiMajorAxisAsFixed() * sbody->GetSemiMajorAxisAsFixed()) * MOON_TIDAL_LOCK;
	}
	//Output("tidal lock of %s: %.5f, a %.5f R %.4f mp %.3f ms %.3f\n", name.c_str(),
	//		invTidalLockTime.ToFloat(), semiMajorAxis.ToFloat(), radius.ToFloat(), parent->mass.ToFloat(), mass.ToFloat());

	if (invTidalLockTime > 10) { // 10x faster than Moon, no chance not to be tidal-locked
		sbody->m_rotationPeriod = fixed(int(round(sbody->GetOrbit().Period())), 3600 * 24);
		sbody->m_axialTilt = sbody->GetInclinationAsFixed();
	} else if (invTidalLockTime > fixed(1, 100)) { // rotation speed changed in favour of tidal lock
		// XXX: there should be some chance the satellite was captured only recently and ignore this
		//		I'm omitting that now, I do not want to change the Universe by additional rand call.

		fixed lambda = invTidalLockTime / (fixed(1, 20) + invTidalLockTime);
		sbody->m_rotationPeriod = (1 - lambda) * sbody->GetRotationPeriodAsFixed() + lambda * sbody->GetOrbit().Period() / 3600 / 24;
		sbody->m_axialTilt = (1 - lambda) * sbody->GetAxialTiltAsFixed() + lambda * sbody->GetInclinationAsFixed();
	} // else .. nothing happens to the satellite

	sbody->SetAtmFromParameters();

	PickAtmosphere(sbody);
	PickRings(sbody);
}

static fixed mass_from_disk_area(fixed a, fixed b, fixed max)
{
	// so, density of the disk with distance from star goes like so: 1 - x/discMax
	//
	// ---
	//    ---
	//       --- <- zero at discMax
	//
	// Which turned into a disc becomes 2*pi*x - (2*pi*x*x)/discMax
	// Integral of which is: pi*x*x - (2/(3*discMax))*pi*x*x*x
	//
	// Because get_disc_density divides total_mass by
	// mass_from_disk_area(0, discMax, discMax) to find density, the
	// constant factors (pi) in this equation drop out.
	//
	b = (b > max ? max : b);
	assert(b >= a);
	assert(a <= max);
	assert(b <= max);
	assert(a >= 0);
	fixed one_over_3max = fixed(2, 1) / (3 * max);

	// We have to avoid overflow of fixed-point numbers
	// Find a representation that doesn't calculate x*x*x
	// m = 2/(3 * discMax)
	// a' = a^2 - m * a^3
	// a' a^-2 = a^-2 ( a^2 - m a^3 ) -> a^2 a^-2 - m a^3 a^-2
	// a' a^-2 = 1 - m a

	// return (b * b - one_over_3max * b * b * b) -
	// 	(a * a - one_over_3max * a * a * a);

	fixed one_max_a = fixed(1, 1) - one_over_3max * a;
	fixed one_max_b = fixed(1, 1) - one_over_3max * b;
	return (b * b * one_max_b) - (a * a * one_max_a);
}

static fixed get_disc_density(SystemBody *primary, fixed discMin, fixed discMax, fixed percentOfPrimaryMass)
{
	discMax = std::max(discMax, discMin + fixed(1, 100)); // avoid divide-by-zero
	fixed total = mass_from_disk_area(discMin, discMax, discMax);
	return primary->GetMassInEarths() * percentOfPrimaryMass / total;
}

static inline bool test_overlap(const fixed &x1, const fixed &x2, const fixed &y1, const fixed &y2)
{
	return (x1 >= y1 && x1 <= y2) ||
		(x2 >= y1 && x2 <= y2) ||
		(y1 >= x1 && y1 <= x2) ||
		(y2 >= x1 && y2 <= x2);
}

fixed StarSystemRandomGenerator::CalcBodySatelliteShellDensity(Random &rand, SystemBody *primary, fixed &discMin, fixed &discMax)
{
	SystemBody::BodySuperType parentSuperType = primary->GetSuperType();

	if (parentSuperType <= SystemBody::SUPERTYPE_STAR) {
		if (primary->GetType() == SystemBody::TYPE_GRAVPOINT) {
			/* around a binary */
			if (primary->HasChildren())
				discMin = primary->m_children[0]->m_orbMax * SAFE_DIST_FROM_BINARY;
			/* empty gravpoint, should only be encountered while creating custom system */
			else
				discMin = fixed(1, 1);
		} else {
			/* correct thing is roche limit, but lets ignore that because
			 * it depends on body densities and gives some strange results */
			discMin = 4 * primary->GetRadiusAsFixed() * AU_SOL_RADIUS;
		}

		if (primary->GetType() == SystemBody::TYPE_BROWN_DWARF) {
			// Increase the minimum radius around brown dwarf stars
			discMin = 100 * primary->GetRadiusAsFixed() * AU_SOL_RADIUS;
		}

		if (primary->GetType() == SystemBody::TYPE_WHITE_DWARF) {
			// white dwarfs will have started as stars < 8 solar
			// masses or so, so pick discMax according to that
			// We give it a larger discMin because it used to be a much larger star
			discMin = 1000 * primary->GetRadiusAsFixed() * AU_SOL_RADIUS;
			discMax = 100 * rand.NFixed(2); // rand-splitting again
			discMax *= fixed::SqrtOf(fixed(1, 2) + fixed(8, 1) * rand.Fixed());
		} else {
			discMax = 100 * rand.NormFixed().Abs() * fixed::SqrtOf(primary->GetMassAsFixed());
		}

		// having limited discMin by bin-separation/fake roche, and
		// discMax by some relation to star mass, we can now compute
		// disc density
		fixed discDensity = get_disc_density(primary, discMin, discMax, fixed(1, 100));

		// Avoid very small, dense stars creating unnatural amounts of gas giants surrounding
		discDensity *= std::min(primary->GetRadiusAsFixed() / primary->GetMassAsFixed(), fixed(1,1));

		// NOTE: limits applied here scale the density distribution function so
		// that bodies are naturally of low mass at the binary/trinary limit

		if ((parentSuperType == SystemBody::SUPERTYPE_STAR) && (primary->m_parent)) {
			// limit planets out to 10% distance to star's binary companion
			discMax = std::min(discMax, primary->m_orbMin * fixed(1, 10));
		}

		/* in trinary and quaternary systems don't bump into other pair... */
		StarSystem *system = primary->GetStarSystem();
		if (system->GetNumStars() >= 3) {
			discMax = std::min(discMax, fixed(5, 100) * system->GetRootBody()->GetChildren()[0]->m_orbMin);
		}

		return discDensity;

	} else {
		fixed primary_rad = primary->GetRadiusAsFixed() * AU_EARTH_RADIUS;
		discMin = 4 * primary_rad;
		discMax = fixed(5000, 1);
		// use hill radius to find max size of moon system. for stars botch it.
		// And use planets orbit around its primary as a scaler to a moon's orbit

		// assume satellites only exist max 1/10th of L1 distance
		// generated value should be well within precision limits
		// NOTE: this is opinionated and serves to limit "useless moons" for
		// gameplay purposes instead of fully representing reality
		fixedf<48> hillSphereRad = CalcHillRadius(primary) * fixedf<48>(1, 4);
		discMax = std::min(discMax, fixed(hillSphereRad));

		return get_disc_density(primary, discMin, discMax, fixed(1, 500));
	}
}

void StarSystemRandomGenerator::MakePlanetsAround(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *primary, Random &rand)
{
	PROFILE_SCOPED()
	SystemBody::BodySuperType parentSuperType = primary->GetSuperType();

	// NOTE: using a consistent seed value here as body shell density should be immutable across multiple invocations
	const SystemPath &path = system->GetPath();
	Random rng { BODY_SATELLITE_SALT, primary->GetSeed(), uint32_t(path.sectorX), uint32_t(path.sectorY), uint32_t(path.sectorZ), UNIVERSE_SEED };

	fixed discMin;
	fixed discMax;
	fixed discDensity = CalcBodySatelliteShellDensity(rng, primary, discMin, discMax);

	if (discMin > discMax || discDensity <= 0)
		return; // can't make planets here, outside of Hill radius

	// Output("Around %s: Range %f -> %f AU, Density %g\n", primary->GetName().c_str(), discMin.ToDouble(), discMax.ToDouble(), discDensity.ToDouble());

	fixed flatJump = parentSuperType == SystemBody::SUPERTYPE_STAR ?
		(primary->GetRadiusAsFixed() * 10) * AU_SOL_RADIUS :
		(primary->GetRadiusAsFixed() * 16) * AU_EARTH_RADIUS;
	fixed initialJump = rand.NFixed(5) * discMax;
	fixed pos = discMin + rand.NormFixed(fixed(3, 1), fixed(25, 10)) * flatJump + initialJump;
	const RingStyle &ring = primary->GetRings();
	const bool hasRings = primary->HasRings();

	assert(pos >= 0);

	// Generating a body can fail if there is a small distance between pos and discMax
	uint32_t numTries = 0;

	while (pos < discMax && numTries++ < 30) {
		SystemBody *planet = MakeBodyInOrbitSlice(rand, system.Get(), primary, pos, fixed(0), discMax, discDensity);

		if (!planet)
			continue;

		primary->m_children.push_back(planet);

		fixed periapsis = planet->m_orbMin;
		fixed apoapsis = planet->m_orbMax;

		if (hasRings &&
			parentSuperType == SystemBody::SUPERTYPE_ROCKY_PLANET &&
			test_overlap(ring.minRadius, ring.maxRadius, periapsis, apoapsis)) {
			//Output("Overlap, eliminating rings from parent SystemBody\n");
			//Overlap, eliminating rings from parent SystemBody
			primary->m_rings.minRadius = fixed();
			primary->m_rings.maxRadius = fixed();
			primary->m_rings.baseColor = Color(255, 255, 255, 255);
		}

		/* minimum separation between planets of 1.2x */
		pos = apoapsis * (rand.NFixed(3) + fixed(12, 10));
	}

	int idx = 0;
	bool make_moons = parentSuperType <= SystemBody::SUPERTYPE_STAR;

	for (std::vector<SystemBody *>::iterator i = primary->m_children.begin(); i != primary->m_children.end(); ++i) {
		// planets around a binary pair [gravpoint] -- ignore the stars...
		if ((*i)->GetSuperType() == SystemBody::SUPERTYPE_STAR) continue;
		// Turn them into something!!!!!!!
		char buf[16];
		if (parentSuperType <= SystemBody::SUPERTYPE_STAR) {
			// planet naming scheme
			snprintf(buf, sizeof(buf), " %c", 'a' + idx);
		} else {
			// moon naming scheme
			snprintf(buf, sizeof(buf), " %d", 1 + idx);
		}
		(*i)->m_name = primary->GetName() + buf;
		PickPlanetType(*i, rand);
		if (make_moons) MakePlanetsAround(system, *i, rand);
		idx++;
	}
}

SystemBody *StarSystemRandomGenerator::MakeBodyInOrbitSlice(Random &rand, StarSystem::GeneratorAPI *system, SystemBody *primary, fixed min_slice, fixed max_slice, fixed discMax, fixed discDensity)
{
	fixed semiMajorAxis;
	fixed eccentricity;

	if (max_slice != 0) {
		// calculate apoapsis to fit within the given orbital slice

		fixed periapsis = min_slice + (max_slice - min_slice) * fixed(1, 2) * rand.NFixed(2);
		fixed apoapsis = max_slice - (max_slice - min_slice) * fixed(1, 2) * rand.NFixed(3);

		// help avoid overflow by calculating this way instead of (ap + pe) * 0.5
		semiMajorAxis = periapsis + (apoapsis - periapsis) * fixed(1, 2);

		// rMax = a(1 + e) -> e = rMax / a - 1
		eccentricity = apoapsis / semiMajorAxis - fixed(1, 1);

	} else {
		// Calculate a random orbit greater than pos and smaller than discMax

		fixed slice_bump = min_slice * fixed(1, 2);

		// Increment the initial periapsis range by a value that falls off the
		// further towards the disc edge we are if orbiting a star
		if (primary->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
			fixed bump_factor = (fixed(1, 1) - min_slice / discMax);
			slice_bump += bump_factor * bump_factor * min_slice;
		}

		// periapsis, apoapsis = closest, farthest distance in orbit
		fixed periapsis = min_slice + slice_bump * rand.NormFixed().Abs();

		if (periapsis > discMax)
			return nullptr;

		// the closer the orbit is to the primary, the higher chance of a regular, concentric orbit
		fixed ecc_factor = fixed(1, 1) - periapsis / discMax;
		ecc_factor *= ecc_factor;

		eccentricity = rand.NFixed(3) * (fixed(1, 1) - ecc_factor);
		semiMajorAxis = periapsis / (fixed(1, 1) - eccentricity);

		fixed apoapsis = 2 * semiMajorAxis - periapsis;
		if (apoapsis > discMax)
			return nullptr;

		max_slice = fixed(135, 100) * apoapsis;
	}

	// reduce disc area for bodies with highly elliptical orbits
	fixed inv_eccentricity = fixed(1,1) - eccentricity;
	fixed max_slice_eff = min_slice + (max_slice - min_slice) * (inv_eccentricity * inv_eccentricity);

	// random mass averaging ~1/2 the density distribution for this slice
	fixed mass = mass_from_disk_area(min_slice, max_slice_eff, discMax);

	// effective planetary mass grows between the primary and this point, and falls from this point to the disc edge
	fixed inner_point = discMax * fixed(1, 3);

	// reduce mass of bodies between 0 .. 0.3(discMax)
	fixed inner_factor = std::min(semiMajorAxis / inner_point, fixed(1, 1));

	// squared falloff to disc edge
	fixed outer_factor = std::max(semiMajorAxis - inner_point, fixed(0)) / (discMax - inner_point);
	outer_factor = fixed(1, 1) - outer_factor * outer_factor;

	mass *= rand.NormFixed(fixed(1, 2), fixed(1, 2)) * inner_factor * outer_factor;

	mass *= discDensity;

	if (mass.v < 0) { // hack around overflow
		Output("WARNING: planetary mass has overflowed! (child %d of %s)\n", primary->GetNumChildren(), primary->GetName().c_str());
		mass = fixed(Sint64(0x7fFFffFFffFFffFFull));
	}

	SystemBody *planet = system->NewBody();
	planet->m_semiMajorAxis = semiMajorAxis;
	planet->m_eccentricity = eccentricity;
	planet->m_axialTilt = fixed(100, 157) * rand.NFixed(2);
	planet->m_type = SystemBody::TYPE_PLANET_TERRESTRIAL;
	planet->m_seed = rand.Int32();
	planet->m_parent = primary;
	planet->m_mass = mass;
	planet->m_rotationPeriod = fixed(rand.Int32(1, 200), 24);

	// longitude of ascending node
	planet->m_orbitalOffset = rand.Fixed() * 2 * FIXED_PI;
	// inclination in the hemisphere above the equator, low probability of high-inclination orbits
	fixed incl_scale = rand.Fixed() * fixed(666, 1000);
	planet->m_inclination = rand.NormFixed().Abs() * incl_scale * FIXED_PI * fixed(1, 2);
	// argument of periapsis, interval -PI .. PI
	planet->m_argOfPeriapsis = rand.NormFixed() * FIXED_PI;

	// rare chance of reversed orbit
	if (rand.Fixed() < fixed(1, 20))
		planet->m_inclination = FIXED_PI - planet->m_inclination;

	// true anomaly as rotation beyond periapsis
	planet->m_orbitalPhaseAtStart = rand.Fixed() * 2 * FIXED_PI;

	planet->SetOrbitFromParameters();
	planet->SetAtmFromParameters();

	return planet;
}

void StarSystemRandomGenerator::MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type, Random &rand)
{
	PROFILE_SCOPED()
	sbody->m_type = type;
	sbody->m_seed = rand.Int32();
	sbody->m_radius = fixed(rand.Int32(starTypeInfo[type].radius[0], starTypeInfo[type].radius[1]), 100);

	// Assign aspect ratios caused by equatorial bulges due to rotation. See terrain code for details.
	// XXX to do: determine aspect ratio distributions for dimmer stars. Make aspect ratios consistent with rotation speeds/stability restrictions.
	switch (type) {
	// Assign aspect ratios (roughly) between 1.0 to 1.8 with a bias towards 1 for bright stars F, A, B ,O

	// "A large fraction of hot stars are rapid rotators with surface rotational velocities
	// of more than 100 km/s (6, 7). ." Imaging the Surface of Altair, John D. Monnier, et. al. 2007
	// A reasonable amount of lot of stars will be assigned high aspect ratios.

	// Bright stars whose equatorial to polar radius ratio (the aspect ratio) is known
	// seem to tend to have values between 1.0 and around 1.5 (brief survey).
	// The limiting factor preventing much higher values seems to be stability as they
	// are rotating 80-95% of their breakup velocity.
	case SystemBody::TYPE_STAR_F:
	case SystemBody::TYPE_STAR_F_GIANT:
	case SystemBody::TYPE_STAR_F_HYPER_GIANT:
	case SystemBody::TYPE_STAR_F_SUPER_GIANT:
	case SystemBody::TYPE_STAR_A:
	case SystemBody::TYPE_STAR_A_GIANT:
	case SystemBody::TYPE_STAR_A_HYPER_GIANT:
	case SystemBody::TYPE_STAR_A_SUPER_GIANT:
	case SystemBody::TYPE_STAR_B:
	case SystemBody::TYPE_STAR_B_GIANT:
	case SystemBody::TYPE_STAR_B_SUPER_GIANT:
	case SystemBody::TYPE_STAR_B_WF:
	case SystemBody::TYPE_STAR_O:
	case SystemBody::TYPE_STAR_O_GIANT:
	case SystemBody::TYPE_STAR_O_HYPER_GIANT:
	case SystemBody::TYPE_STAR_O_SUPER_GIANT:
	case SystemBody::TYPE_STAR_O_WF: {
		fixed rnd = rand.Fixed();
		sbody->m_aspectRatio = fixed(1, 1) + fixed(8, 10) * rnd * rnd;
		break;
	}
	// aspect ratio is initialised to 1.0 for other stars currently
	default:
		break;
	}
	sbody->m_mass = fixed(rand.Int32(starTypeInfo[type].mass[0], starTypeInfo[type].mass[1]), 100);
	sbody->m_averageTemp = rand.Int32(starTypeInfo[type].tempMin, starTypeInfo[type].tempMax);
}

void StarSystemRandomGenerator::MakeRandomStar(SystemBody *sbody, Random &rand)
{
	PROFILE_SCOPED()
	SystemBody::BodyType type = SystemBody::BodyType(rand.Int32(SystemBody::TYPE_STAR_MIN, SystemBody::TYPE_STAR_MAX));
	MakeStarOfType(sbody, type, rand);
}

void StarSystemRandomGenerator::MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass, Random &rand)
{
	PROFILE_SCOPED()
	int tries = 16;
	do {
		MakeStarOfType(sbody, type, rand);
	} while ((sbody->GetMassAsFixed() > maxMass) && (--tries));
}

void StarSystemRandomGenerator::MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist, Random &rand)
{
	PROFILE_SCOPED()
	fixed m = a->GetMassAsFixed() + b->GetMassAsFixed();
	fixed a0 = b->GetMassAsFixed() / m;
	fixed a1 = a->GetMassAsFixed() / m;
	a->m_eccentricity = rand.NFixed(3);
	int mul = 1;

	do {
		switch (rand.Int32(3)) {
		case 2: a->m_semiMajorAxis = fixed(rand.Int32(100, 10000), 100); break;
		case 1: a->m_semiMajorAxis = fixed(rand.Int32(10, 1000), 100); break;
		default:
		case 0: a->m_semiMajorAxis = fixed(rand.Int32(1, 100), 100); break;
		}
		a->m_semiMajorAxis *= mul;
		mul *= 2;
	} while (a->m_semiMajorAxis - a->m_eccentricity * a->m_semiMajorAxis < minDist);

	const double total_mass = a->GetMass() + b->GetMass();
	const double e = a->m_eccentricity.ToDouble();

	a->m_orbit.SetShapeAroundBarycentre(AU * (a->m_semiMajorAxis * a0).ToDouble(), total_mass, a->GetMass(), e);
	b->m_orbit.SetShapeAroundBarycentre(AU * (a->m_semiMajorAxis * a1).ToDouble(), total_mass, b->GetMass(), e);

	const float rotX = -0.5f * float(M_PI); //(float)(rand.Double()*M_PI/2.0);
	const float rotY = static_cast<float>(rand.Double(M_PI));
	a->m_orbit.SetPlane(matrix3x3d::RotateY(rotY) * matrix3x3d::RotateX(rotX));
	b->m_orbit.SetPlane(matrix3x3d::RotateY(rotY - M_PI) * matrix3x3d::RotateX(rotX));

	// store orbit parameters for later use to be accessible in other way than by rotMatrix
	b->m_orbitalPhaseAtStart = b->m_orbitalPhaseAtStart + FIXED_PI;
	b->m_orbitalPhaseAtStart = b->m_orbitalPhaseAtStart > 2 * FIXED_PI ? b->m_orbitalPhaseAtStart - 2 * FIXED_PI : b->m_orbitalPhaseAtStart;
	a->m_orbitalPhaseAtStart = a->m_orbitalPhaseAtStart > 2 * FIXED_PI ? a->m_orbitalPhaseAtStart - 2 * FIXED_PI : a->m_orbitalPhaseAtStart;
	a->m_orbitalPhaseAtStart = a->m_orbitalPhaseAtStart < 0 ? a->m_orbitalPhaseAtStart + 2 * FIXED_PI : a->m_orbitalPhaseAtStart;
	b->m_orbitalOffset = fixed(int(round(rotY * 10000)), 10000);
	a->m_orbitalOffset = fixed(int(round(rotY * 10000)), 10000);

	fixed orbMin = a->m_semiMajorAxis - a->m_eccentricity * a->m_semiMajorAxis;
	fixed orbMax = 2 * a->m_semiMajorAxis - orbMin;
	a->m_orbMin = orbMin;
	b->m_orbMin = orbMin;
	a->m_orbMax = orbMax;
	b->m_orbMax = orbMax;
}

bool StarSystemRandomGenerator::Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config)
{
	PROFILE_SCOPED()
	RefCountedPtr<const Sector> sec = galaxy->GetSector(system->GetPath());
	const Sector::System &secSys = sec->m_systems[system->GetPath().systemIndex];

	if (config->isCustomOnly)
		return true;

	SystemBody *star[4];
	SystemBody *centGrav1(0), *centGrav2(0);

	const int numStars = secSys.GetNumStars();
	assert((numStars >= 1) && (numStars <= 4));
	if (numStars == 1) {
		SystemBody::BodyType type = sec->m_systems[system->GetPath().systemIndex].GetStarType(0);
		star[0] = system->NewBody();
		star[0]->m_parent = 0;
		star[0]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName();
		star[0]->m_orbMin = fixed();
		star[0]->m_orbMax = fixed();

		MakeStarOfType(star[0], type, rng);
		system->SetRootBody(star[0]);
		system->SetNumStars(1);
	} else {
		centGrav1 = system->NewBody();
		centGrav1->m_type = SystemBody::TYPE_GRAVPOINT;
		centGrav1->m_parent = 0;
		centGrav1->m_name = sec->m_systems[system->GetPath().systemIndex].GetName() + " A,B";
		system->SetRootBody(centGrav1);

		SystemBody::BodyType type = sec->m_systems[system->GetPath().systemIndex].GetStarType(0);
		star[0] = system->NewBody();
		star[0]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName() + " A";
		star[0]->m_parent = centGrav1;
		MakeStarOfType(star[0], type, rng);

		star[1] = system->NewBody();
		star[1]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName() + " B";
		star[1]->m_parent = centGrav1;
		MakeStarOfTypeLighterThan(star[1], sec->m_systems[system->GetPath().systemIndex].GetStarType(1), star[0]->GetMassAsFixed(), rng);

		centGrav1->m_mass = star[0]->GetMassAsFixed() + star[1]->GetMassAsFixed();
		centGrav1->m_children.push_back(star[0]);
		centGrav1->m_children.push_back(star[1]);
		// Separate stars by 0.2 radii for each, so that their planets don't bump into the other star
		const fixed minDist1 = (fixed(12, 10) * star[0]->GetRadiusAsFixed() + fixed(12, 10) * star[1]->GetRadiusAsFixed()) * AU_SOL_RADIUS;
	try_that_again_guvnah:
		MakeBinaryPair(star[0], star[1], minDist1, rng);

		system->SetNumStars(2);

		if (numStars > 2) {
			if (star[0]->m_orbMax > fixed(100, 1)) {
				// reduce to < 100 AU...
				goto try_that_again_guvnah;
			}
			// 3rd and maybe 4th star
			if (numStars == 3) {
				star[2] = system->NewBody();
				star[2]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName() + " C";
				star[2]->m_orbMin = 0;
				star[2]->m_orbMax = 0;
				MakeStarOfTypeLighterThan(star[2], sec->m_systems[system->GetPath().systemIndex].GetStarType(2), star[0]->GetMassAsFixed(), rng);
				centGrav2 = star[2];
				system->SetNumStars(3);
			} else {
				centGrav2 = system->NewBody();
				centGrav2->m_type = SystemBody::TYPE_GRAVPOINT;
				centGrav2->m_name = sec->m_systems[system->GetPath().systemIndex].GetName() + " C,D";
				centGrav2->m_orbMax = 0;

				star[2] = system->NewBody();
				star[2]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName() + " C";
				star[2]->m_parent = centGrav2;
				MakeStarOfTypeLighterThan(star[2], sec->m_systems[system->GetPath().systemIndex].GetStarType(2), star[0]->GetMassAsFixed(), rng);

				star[3] = system->NewBody();
				star[3]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName() + " D";
				star[3]->m_parent = centGrav2;
				MakeStarOfTypeLighterThan(star[3], sec->m_systems[system->GetPath().systemIndex].GetStarType(3), star[2]->GetMassAsFixed(), rng);

				// Separate stars by 0.2 radii for each, so that their planets don't bump into the other star
				const fixed minDist2 = (fixed(12, 10) * star[2]->GetRadiusAsFixed() + fixed(12, 10) * star[3]->GetRadiusAsFixed()) * AU_SOL_RADIUS;
				MakeBinaryPair(star[2], star[3], minDist2, rng);
				centGrav2->m_mass = star[2]->GetMassAsFixed() + star[3]->GetMassAsFixed();
				centGrav2->m_children.push_back(star[2]);
				centGrav2->m_children.push_back(star[3]);
				system->SetNumStars(4);
			}
			SystemBody *superCentGrav = system->NewBody();
			superCentGrav->m_type = SystemBody::TYPE_GRAVPOINT;
			superCentGrav->m_parent = 0;
			superCentGrav->m_name = sec->m_systems[system->GetPath().systemIndex].GetName();
			centGrav1->m_parent = superCentGrav;
			centGrav2->m_parent = superCentGrav;
			system->SetRootBody(superCentGrav);
			const fixed minDistSuper = star[0]->m_orbMax + star[2]->m_orbMax;
			MakeBinaryPair(centGrav1, centGrav2, 4 * minDistSuper, rng);
			superCentGrav->m_children.push_back(centGrav1);
			superCentGrav->m_children.push_back(centGrav2);
		}
	}

	// used in MakeShortDescription
	// XXX except this does not reflect the actual mining happening in this system
	system->SetMetallicity(starMetallicities[system->GetRootBody()->GetType()]);

	// store all of the stars first ...
	for (uint32_t i = 0; i < system->GetNumStars(); i++) {
		system->AddStar(star[i]);
	}
	// ... because we need them when making planets to calculate surface temperatures
	for (auto *s : system->GetStars()) {
		MakePlanetsAround(system, s, rng);
	}

	if (system->GetNumStars() > 1)
		MakePlanetsAround(system, centGrav1, rng);
	if (system->GetNumStars() == 4)
		MakePlanetsAround(system, centGrav2, rng);

		// an example export of generated system, can be removed during the merge
		//char filename[500];
		//snprintf(filename, 500, "tmp-sys/%s.lua", GetName().c_str());
		//ExportToLua(filename);

#ifdef DEBUG_DUMP
	Dump();
#endif /* DEBUG_DUMP */
	return true;
}

/*
 * Position a surface starport anywhere. Space.cpp::MakeFrameFor() ensures it
 * is on dry land (discarding this position if necessary)
 */
void PopulateStarSystemGenerator::PositionSettlementOnPlanet(SystemBody *sbody, std::vector<fixed> &prevOrbits)
{
	PROFILE_SCOPED()
	Random r(sbody->GetSeed());
	// used for orientation on planet surface

	fixed longitude = r.Fixed(); // function parameter evaluation order is implementation-dependent
	fixed latitude = r.NormFixed();  // can't put two rands in the same expression

	// try to ensure that stations are far enough apart

	for (size_t i = 0, iterations = 0; i < prevOrbits.size() && iterations < 128; i++, iterations++) {
		const fixed &prev = prevOrbits[i];
		const fixed len = (latitude - prev).Abs();
		if (len < fixed(5, 100)) {
			longitude = r.Fixed();
			latitude = r.NormFixed();
			i = 0; // reset to start the checking from beginning as we're generating new values.
		}
	}
	prevOrbits.push_back(latitude.ToDouble());

	// store latitude and longitude to equivalent orbital parameters to
	// be accessible easier
	sbody->m_inclination = latitude * FIXED_PI * fixed(1, 2);
	sbody->m_orbitalOffset = longitude * FIXED_PI * 2;

	sbody->SetOrbitFromParameters();
}

/*
 * Set natural resources, tech level, industry strengths and population levels
 */
void PopulateStarSystemGenerator::PopulateStage1(SystemBody *sbody, StarSystem::GeneratorAPI *system, fixed &outTotalPop)
{
	PROFILE_SCOPED()
	for (auto *child : sbody->GetChildren()) {
		PopulateStage1(child, system, outTotalPop);
	}

	// unexplored systems have no population (that we know about)
	if (system->GetExplored() != StarSystem::eEXPLORED_AT_START) {
		sbody->m_population = outTotalPop = fixed();
		return;
	}

	// grav-points have no population themselves
	if (sbody->GetType() == SystemBody::TYPE_GRAVPOINT) {
		sbody->m_population = fixed();
		return;
	}

	Uint32 _init[6] = { Uint32(system->GetSeed()), Uint32(system->GetPath().sectorX),
		Uint32(system->GetPath().sectorY), Uint32(system->GetPath().sectorZ), UNIVERSE_SEED, Uint32(sbody->GetSeed()) };

	Random rand;
	rand.seed(_init, 6);

	RefCountedPtr<Random> namerand(new Random);
	namerand->seed(_init, 6);

	sbody->m_population = fixed();

	/* Bad type of planet for settlement */
	if ((sbody->GetAverageTemp() > CELSIUS + 100) || (sbody->GetAverageTemp() < 100) || (sbody->CalcSurfaceGravity() > MAX_SETTLEMENT_SURFACE_GRAVITY) ||
		(sbody->GetType() != SystemBody::TYPE_PLANET_TERRESTRIAL && sbody->GetType() != SystemBody::TYPE_PLANET_ASTEROID)) {
		Random starportPopRand;
		starportPopRand.seed(_init, 6);

		// orbital starports should carry a small amount of population
		if (sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL) {
			// give starports a population between 9000 and 30000
			sbody->m_population = fixed(1, 100000) + fixed(starportPopRand.Int32(-1000, 20000), 1000000000);
			outTotalPop += sbody->m_population;
		} else if (sbody->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
			// No permanent population on gravities larger than defined in MAX_SETTLEMENT_SURFACE_GRAVITY
			if (sbody->CalcSurfaceGravity() > MAX_SETTLEMENT_SURFACE_GRAVITY) {
				outTotalPop = fixed();
			} else {
				// give surface spaceports a population between 80000 and 250000
				sbody->m_population = fixed(1, 10000) + fixed(starportPopRand.Int32(-2000, 15000), 100000000);
				outTotalPop += sbody->m_population;
			}
		}

		return;
	}

	sbody->m_agricultural = fixed();

	if (sbody->GetLifeAsFixed() > fixed(9, 10)) {
		sbody->m_agricultural = Clamp(fixed(1, 1) - fixed(CELSIUS + 25 - sbody->GetAverageTemp(), 40), fixed(), fixed(1, 1));
		system->SetAgricultural(system->GetAgricultural() + 2 * sbody->m_agricultural);
	} else if (sbody->GetLifeAsFixed() > fixed(1, 2)) {
		sbody->m_agricultural = Clamp(fixed(1, 1) - fixed(CELSIUS + 30 - sbody->GetAverageTemp(), 50), fixed(), fixed(1, 1));
		system->SetAgricultural(system->GetAgricultural() + 1 * sbody->m_agricultural);
	} else {
		// don't bother populating crap planets
		if (sbody->GetMetallicityAsFixed() < fixed(5, 10) &&
			sbody->GetMetallicityAsFixed() < (fixed(1, 1) - system->GetHumanProx())) return;
	}

	/* Commodities we produce (mining and agriculture) */

	fixed workforce = fixed();
	for (const auto &commodity : GalacticEconomy::Commodities()) {
		const GalacticEconomy::EconomyInfo &economy = GalacticEconomy::GetEconomyById(commodity.producer);

		fixed affinity = fixed();
		int numFactors = 0;
		// TODO(sturnclaw): this is a horrible way of determining commodity production
		// Find something better to replace it that actually makes sense.
		if (economy.affinity.agricultural > 0) {
			affinity += economy.affinity.agricultural * sbody->GetAgriculturalAsFixed();
			numFactors++;
		}
		if (economy.affinity.metallicity > 0) {
			affinity += economy.affinity.metallicity * sbody->GetMetallicityAsFixed();
			numFactors++;
		}
		if (economy.affinity.industrial > 0) {
			affinity += economy.affinity.industrial * system->GetIndustrial();
			numFactors++;
		}

		if (numFactors)
			affinity /= numFactors;
		else
			affinity = fixed(1, 1);

		affinity *= rand.Fixed();

		if (GalacticEconomy::Consumables().count(commodity.id)) {
			affinity *= 2;
		}

		/* workforce... */
		workforce += affinity * system->GetHumanProx();

		// TODO(sturnclaw): this is also a horrible way of determining commodity production.
		// Commodity consumption / production should be based on actual numbers rather than
		// arbitrary percentage price reduction.
		fixed howmuch = affinity * 256;

		// TODO(sturnclaw): quick and dirty patch to provide a bit more variance in galactic
		// economy demand. This reduces the 'global' dependence on certain input commodities
		// and allows a system to conceptually produce more of the commodity than it consumes
		howmuch = howmuch * rand.Fixed() * 3;

		if (howmuch.ToInt32() == 0)
			continue;

		// Produce X amount of this commodity
		system->AddTradeLevel(commodity.id, -2 * howmuch.ToInt32());
		// Output("System %s.%s adding commodity %s (%d) -> %d\n",
		// 	system->GetName(), sbody->GetName(), commodity.name, -2 * howmuch.ToInt32(), system->GetTradeLevel(commodity.id));
		for (const auto &input : commodity.inputs) {
			// Consume Y amount of the input commodities
			system->AddTradeLevel(input.first, (input.second * howmuch).ToInt32());
		}
	}

	sbody->m_population += workforce;

	if (!system->HasCustomBodies() && sbody->GetPopulationAsFixed() > 0)
		sbody->m_name = Pi::luaNameGen->BodyName(sbody, namerand);

	// Add a bunch of things people consume
	for (const auto &pair : GalacticEconomy::Consumables()) {
		const GalacticEconomy::ConsumableInfo &consumable = pair.second;

		// Some consumables are assumed to be produced locally if the planet is suitably populated.
		if (sbody->GetLifeAsFixed() > consumable.locally_produced_min)
			continue;

		const auto &consume_bounds = consumable.random_consumption;
		uint32_t consumption = rand.Int32(consume_bounds[0], consume_bounds[1]);
		system->AddTradeLevel(pair.first, consumption);

		// auto &commodity = GalacticEconomy::GetCommodityById(t.first);
		// Output("System %s.%s adding consumable %s (%d) -> %d\n",
		// 	system->GetName(), sbody->GetName(), commodity.name, consumption, system->GetTradeLevel(commodity.id));
	}

	// well, outdoor worlds should have way more people
	sbody->m_population = fixed(1, 10) * sbody->m_population + sbody->m_population * sbody->GetAgriculturalAsFixed();

	//	Output("%s: pop %.3f billion\n", name.c_str(), sbody->m_population.ToFloat());

	outTotalPop += sbody->GetPopulationAsFixed();
}

static bool check_unique_station_name(const std::string &name, const StarSystem *system)
{
	PROFILE_SCOPED()
	bool ret = true;
	for (const SystemBody *station : system->GetSpaceStations())
		if (station->GetName() == name) {
			ret = false;
			break;
		}
	return ret;
}

static std::string gen_unique_station_name(SystemBody *sp, const StarSystem *system, RefCountedPtr<Random> &namerand)
{
	PROFILE_SCOPED()
	std::string name;
	do {
		name = Pi::luaNameGen->BodyName(sp, namerand);
	} while (!check_unique_station_name(name, system));
	return name;
}

void PopulateStarSystemGenerator::PopulateAddStations(SystemBody *sbody, StarSystem::GeneratorAPI *system)
{
	PROFILE_SCOPED()
	for (auto *child : sbody->GetChildren())
		PopulateAddStations(child, system);

	Uint32 _init[6] = { Uint32(system->GetSeed()), Uint32(system->GetPath().sectorX),
		Uint32(system->GetPath().sectorY), Uint32(system->GetPath().sectorZ), sbody->GetSeed(), UNIVERSE_SEED };

	Random rand;
	rand.seed(_init, 6);

	RefCountedPtr<Random> namerand(new Random);
	namerand->seed(_init, 6);

	// XXX: station placement code is in need of improvement; the code currently fails to appear "intelligent"
	// with respect to well-spaced orbital shells (e.g. a "station belt" around a high-population planet)
	// as well as generating station placement for e.g. research, industrial, or communications stations

	if (sbody->GetPopulationAsFixed() < fixed(1, 1000)) return;
	fixed orbMaxS = fixed(1, 4) * fixed(CalcHillRadius(sbody));
	fixed orbMinS = fixed().FromDouble((sbody->CalcAtmosphereParams().atmosRadius + +500000.0 / EARTH_RADIUS)) * AU_EARTH_RADIUS;
	if (sbody->GetNumChildren() > 0)
		orbMaxS = std::min(orbMaxS, fixed(1, 2) * sbody->GetChildren()[0]->GetOrbMinAsFixed());

	// starports - orbital
	fixed pop = sbody->GetPopulationAsFixed() + rand.Fixed();
	if (orbMinS < orbMaxS) {

		// How many stations do we need?
		pop -= rand.Fixed();
		Uint32 NumToMake = 0;
		while (pop >= 0) {
			++NumToMake;
			pop -= fixed(1, 1) - rand.NormFixed().Abs();
		}

		// Always generate a station around a populated high-gravity world
		if ((NumToMake == 0) and (sbody->CalcSurfaceGravity() > 10.5)) {  // 10.5 m/s2 = 1,07 g
			NumToMake = 1;
		}

		// Any to position?
		if (NumToMake > 0) {
			const double centralMass = sbody->GetMassAsFixed().ToDouble() * EARTH_MASS;

			// What is our innermost orbit?
			fixed innerOrbit = orbMinS; // + ((orbMaxS - orbMinS) / 25);

			// Try to limit the inner orbit to at least three hours.
			{
				const double minHours = 3.0;
				const double seconds = Orbit::OrbitalPeriod(innerOrbit.ToDouble() * AU, centralMass);
				const double hours = seconds / (60.0 * 60.0);
				if (hours < minHours) {
					//knowing that T=2*pi*R/sqrt(G*M/R) find R for set T=4 hours:
					fixed orbitFromPeriod = fixed().FromDouble((std::pow(G * centralMass, 1.0 / 3.0) * std::pow(minHours * 60.0 * 60.0, 2.0 / 3.0)) / (std::pow(2.0 * M_PI, 2.0 / 3.0) * AU));
					// We can't go higher than our maximum so set it to that.
					innerOrbit = std::min(orbMaxS, orbitFromPeriod);
				}
			}

			// I like to think that we'd fill several "shells" of orbits at once rather than fill one and move out further
			static const Uint32 MAX_ORBIT_SHELLS = 3;
			fixed shells[MAX_ORBIT_SHELLS];
			fixed shellIncl[MAX_ORBIT_SHELLS];

			if (innerOrbit != orbMaxS) {
				shells[0] = innerOrbit;											 // low
				shells[1] = innerOrbit + ((orbMaxS - innerOrbit) * fixed(1, 2)); // med
				shells[2] = orbMaxS;											 // high

				shellIncl[0] = rand.NormFixed() * FIXED_PI;
				shellIncl[1] = rand.NormFixed() * FIXED_PI;
				shellIncl[2] = rand.NormFixed() * FIXED_PI;
			} else {
				shells[0] = shells[1] = shells[2] = innerOrbit;
				shellIncl[0] = shellIncl[1] = shellIncl[2] = rand.NormFixed() * FIXED_PI;
			}

			Uint32 orbitIdx = 0;

			for (Uint32 i = 0; i < NumToMake; i++) {
				// Pick the orbit we've currently placing a station into.
				const fixed currOrbit = shells[orbitIdx];
				const fixed currOrbitIncl = shells[orbitIdx];
				++orbitIdx;
				if (orbitIdx >= MAX_ORBIT_SHELLS) // wrap it
				{
					orbitIdx = 0;
				}

				// Begin creation of the new station
				SystemBody *sp = system->NewBody();
				sp->m_type = SystemBody::TYPE_STARPORT_ORBITAL;
				sp->m_seed = rand.Int32();
				sp->m_parent = sbody;
				sp->m_rotationPeriod = fixed(1, 3600);
				sp->m_averageTemp = sbody->GetAverageTemp();
				sp->m_mass = 0;

				// place stations between min and max orbits to reduce the number of extremely close/fast orbits
				// This avoids the worst-case of having 3-5 stations all in the exact orbit close to each other
				sp->m_semiMajorAxis = currOrbit * rand.NormFixed(fixed(12, 10), fixed(2, 10));

				// Generate slightly random orbits for each station in the orbital "shell"
				// slightly random min/max orbital distance
				sp->m_eccentricity = rand.NormFixed().Abs() * fixed(1, 8);
				// perturb the orbital plane to avoid all stations falling in line with each other
				sp->m_inclination = currOrbitIncl + rand.NormFixed() * fixed(1, 4) * FIXED_PI;
				// station spacing around the primary body
				sp->m_argOfPeriapsis = rand.Fixed() * FIXED_PI * 2;
				// TODO: no axial tilt for stations / axial tilt in general is strangely modeled
				sp->m_axialTilt = fixed();

				sp->SetOrbitFromParameters();

				sbody->m_children.insert(sbody->m_children.begin(), sp);
				system->AddSpaceStation(sp);
				sp->m_orbMin = sp->GetSemiMajorAxisAsFixed();
				sp->m_orbMax = sp->GetSemiMajorAxisAsFixed();

				sp->m_name = gen_unique_station_name(sp, system, namerand);
			}
		}
	}
	// starports - surface
	// give it a fighting chance of having a decent number of starports (*3)
	pop = sbody->GetPopulationAsFixed() + (rand.Fixed() * 3);
	std::vector<fixed> previousOrbits;
	previousOrbits.reserve(8);
	int max = 6;
	while (max-- > 0) {
		pop -= (fixed(1, 1) - rand.NormFixed());
		if (pop < 0) break;

		SystemBody *sp = system->NewBody();
		sp->m_type = SystemBody::TYPE_STARPORT_SURFACE;
		sp->m_seed = rand.Int32();
		sp->m_parent = sbody;
		sp->m_averageTemp = sbody->GetAverageTemp();
		sp->m_mass = 0;
		sp->m_name = gen_unique_station_name(sp, system, namerand);
		sp->m_orbit = Orbit();
		PositionSettlementOnPlanet(sp, previousOrbits);
		sbody->m_children.insert(sbody->m_children.begin(), sp);
		system->AddSpaceStation(sp);
	}

	// guarantee that there is always a star port on a populated world
	if (!system->HasSpaceStations()) {
		SystemBody *sp = system->NewBody();
		sp->m_type = SystemBody::TYPE_STARPORT_SURFACE;
		sp->m_seed = rand.Int32();
		sp->m_parent = sbody;
		sp->m_averageTemp = sbody->m_averageTemp;
		sp->m_mass = 0;
		sp->m_name = gen_unique_station_name(sp, system, namerand);
		sp->m_orbit = Orbit();
		PositionSettlementOnPlanet(sp, previousOrbits);
		sbody->m_children.insert(sbody->m_children.begin(), sp);
		system->AddSpaceStation(sp);
	}
}

void PopulateStarSystemGenerator::SetSysPolit(RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, const fixed &human_infestedness)
{
	SystemPath path = system->GetPath();
	const Uint32 _init[5] = { Uint32(system->GetSeed()), Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), POLIT_SEED };
	Random rand(_init, 5);

	RefCountedPtr<const Sector> sec = galaxy->GetSector(path);
	const CustomSystem *customSystem = sec->m_systems[path.systemIndex].GetCustomSystem();
	SysPolit sysPolit = system->GetSysPolit();

	/* sysPolit should already be populated from custom system definition */
	if (!customSystem)
		sysPolit.govType = Polit::GOV_INVALID;

	if (sysPolit.govType == Polit::GOV_INVALID) {
		if (path == SystemPath(0, 0, 0, 0)) {
			sysPolit.govType = Polit::GOV_EARTHDEMOC;
		} else if (human_infestedness > 0) {
			// attempt to get the government type from the faction
			sysPolit.govType = system->GetFaction()->PickGovType(rand);

			// if that fails, either no faction or a faction with no gov types, then pick something at random
			if (sysPolit.govType == Polit::GOV_INVALID) {
				sysPolit.govType = static_cast<Polit::GovType>(rand.Int32(Polit::GOV_RAND_MIN, Polit::GOV_RAND_MAX));
			}
		} else {
			sysPolit.govType = Polit::GOV_NONE;
		}
	}

	if (!customSystem || customSystem->want_rand_lawlessness)
		sysPolit.lawlessness = Polit::GetBaseLawlessness(sysPolit.govType) * rand.Fixed();

	system->SetSysPolit(sysPolit);
}

void PopulateStarSystemGenerator::SetCommodityLegality(RefCountedPtr<StarSystem::GeneratorAPI> system)
{
	const SystemPath path = system->GetPath();
	const Uint32 _init[5] = { Uint32(system->GetSeed()), Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), POLIT_SALT };
	Random rand(_init, 5);

	// All legal flags were set to true on initialization
	Polit::GovType a = system->GetSysPolit().govType;
	if (a == Polit::GOV_NONE) return;

	if (system->GetFaction()->idx != Faction::BAD_FACTION_IDX) {
		for (const std::pair<const GalacticEconomy::CommodityId, Uint32> &legality : system->GetFaction()->commodity_legality)
			system->SetCommodityLegal(legality.first, (rand.Int32(100) >= legality.second));
	}
	// if this system doesn't have a faction, something's wrong and we assume it's an independent anarchy
}

void PopulateStarSystemGenerator::SetEconType(RefCountedPtr<StarSystem::GeneratorAPI> system)
{
	PROFILE_SCOPED()
	fixed score, maximum = 0;
	GalacticEconomy::EconomyId best_econ_id = GalacticEconomy::InvalidEconomyId;
	// Score each economy typ according to this system's parameters.
	for (const auto &econ : GalacticEconomy::Economies()) {
		score = system->GetAgricultural() * econ.generation.agricultural;
		score += system->GetIndustrial() * econ.generation.industrial;
		score += system->GetMetallicity() * econ.generation.metallicity;
		if (score > maximum) {
			maximum = score;
			best_econ_id = econ.id;
		}
	}
	system->SetEconType(best_econ_id);
}

/* percent */
static const int MAX_COMMODITY_BASE_PRICE_ADJUSTMENT = 25;

bool PopulateStarSystemGenerator::Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system,
	GalaxyGenerator::StarSystemConfig *config)
{
	PROFILE_SCOPED()
	const bool addSpaceStations = !config->isCustomOnly;
	Uint32 _init[5] = { Uint32(system->GetSeed()), Uint32(system->GetPath().sectorX), Uint32(system->GetPath().sectorY), Uint32(system->GetPath().sectorZ), UNIVERSE_SEED };
	Random rand;
	rand.seed(_init, 5);

	/* Various system-wide characteristics */
	// This is 1 in sector (0,0,0) and approaches 0 farther out
	// (1,0,0) ~ .688, (1,1,0) ~ .557, (1,1,1) ~ .48
	system->SetHumanProx(galaxy->GetFactions()->IsHomeSystem(system->GetPath()) ? fixed(2, 3) : fixed(3, 1) / isqrt(9 + 10 * (system->GetPath().sectorX * system->GetPath().sectorX + system->GetPath().sectorY * system->GetPath().sectorY + system->GetPath().sectorZ * system->GetPath().sectorZ)));
	system->SetIndustrial(rand.Fixed());
	system->SetAgricultural(0);

	/* system attributes */
	fixed totalPop = fixed();
	PopulateStage1(system->GetRootBody().Get(), system.Get(), totalPop);
	system->SetTotalPop(totalPop);

	//	Output("Trading rates:\n");
	// So now we have balances of trade of various commodities.
	// Lets use black magic to turn these into percentage base price
	// alterations
	int maximum = 0;
	for (const auto &commodity : GalacticEconomy::Commodities()) {
		maximum = std::max(abs(system->GetTradeLevel(commodity.id)), maximum);
	}
	if (maximum)
		for (const auto &commodity : GalacticEconomy::Commodities()) {
			system->SetTradeLevel(commodity.id, (system->GetTradeLevel(commodity.id) * MAX_COMMODITY_BASE_PRICE_ADJUSTMENT) / maximum);
			system->AddTradeLevel(commodity.id, rand.Int32(-5, 5));
		}

	// Unused?
	//	for (int i=(int)Equip::FIRST_COMMODITY; i<=(int)Equip::LAST_COMMODITY; i++) {
	//		Equip::Type t = (Equip::Type)i;
	//		const EquipType &type = Equip::types[t];
	//		Output("%s: %d%%\n", type.name, m_tradeLevel[t]);
	//	}
	//	Output("System total population %.3f billion\n", m_totalPop.ToFloat());
	SetSysPolit(galaxy, system, system->GetTotalPop());
	SetCommodityLegality(system);

	if (addSpaceStations) {
		PopulateAddStations(system->GetRootBody().Get(), system.Get());
	}

	if (!system->GetShortDescription().size()) {
		SetEconType(system);
		system->MakeShortDescription();
	}

	return true;
}
