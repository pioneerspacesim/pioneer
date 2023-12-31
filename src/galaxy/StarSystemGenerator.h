// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef STARSYSTEM_GENERATOR_H
#define STARSYSTEM_GENERATOR_H

#include "GalaxyGenerator.h"
#include "StarSystem.h"

class StarSystemFromSectorGenerator : public StarSystemGeneratorStage {
public:
	virtual bool Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config);
};

class StarSystemLegacyGeneratorBase : public StarSystemGeneratorStage {
public:
	static void PickAtmosphere(SystemBody *sbody);
	static void PickRings(SystemBodyData *sbody, bool forceRings = false);

protected:
	struct StarTypeInfo {
		int mass[2]; // min,max % sol for stars, unused for planets
		int radius[2]; // min,max % sol radii for stars, % earth radii for planets
		int tempMin, tempMax;
	};
	static const fixed starMetallicities[];
	static const StarTypeInfo starTypeInfo[];

	fixedf<48> CalcHillRadius(SystemBody *sbody) const;
};

class StarSystemCustomGenerator : public StarSystemLegacyGeneratorBase {
public:
	virtual bool Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config);

	// returns true if the system is custom, false if the contents should be randomly generated
	bool ApplyToSystem(Random &rng, RefCountedPtr<StarSystem::GeneratorAPI> system, const CustomSystem *customSys);

private:
	void CustomGetKidsOf(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *parent, const std::vector<CustomSystemBody *> &children, int *outHumanInfestedness);
};

class StarSystemRandomGenerator : public StarSystemLegacyGeneratorBase {
public:
	static constexpr uint32_t BODY_SATELLITE_SALT = 0xf5123a90;

	virtual bool Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config);

	// Calculate the min, max distances from the primary where satellites should be generated
	// Returns the mass density of a 2d slice through the center of the shell
	fixed CalcBodySatelliteShellDensity(Random &rand, SystemBody *primary, fixed &discMin, fixed &discMax);
	SystemBody *MakeBodyInOrbitSlice(Random &rand, StarSystem::GeneratorAPI *system, SystemBody *primary, fixed min, fixed max, fixed discMax, fixed discDensity);
	void PickPlanetType(SystemBody *sbody, Random &rand);

private:
	void MakePlanetsAround(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *primary, Random &rand);
	void MakeRandomStar(SystemBody *sbody, Random &rand);
	void MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type, Random &rand);
	void MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass, Random &rand);
	void MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist, Random &rand);

	int CalcSurfaceTemp(const SystemBody *primary, fixed distToPrimary, fixed albedo, fixed greenhouse);
	const SystemBody *FindStarAndTrueOrbitalRange(const SystemBody *planet, fixed &orbMin_, fixed &orbMax_) const;
};

class PopulateStarSystemGenerator : public StarSystemLegacyGeneratorBase {
public:
	virtual bool Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config);

private:
	void SetSysPolit(RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, const fixed &human_infestedness);
	void SetCommodityLegality(RefCountedPtr<StarSystem::GeneratorAPI> system);
	void SetEconType(RefCountedPtr<StarSystem::GeneratorAPI> system);

	void PopulateAddStations(SystemBody *sbody, StarSystem::GeneratorAPI *system);
	void PositionSettlementOnPlanet(SystemBody *sbody, std::vector<fixed> &prevOrbits);
	void PopulateStage1(SystemBody *sbody, StarSystem::GeneratorAPI *system, fixed &outTotalPop);
};

#endif
