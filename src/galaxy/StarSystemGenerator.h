// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef STARSYSTEM_GENERATOR_H
#define STARSYSTEM_GENERATOR_H

#include "StarSystem.h"
#include "GalaxyGenerator.h"

class StarSystemFromSectorGenerator : public StarSystemGeneratorStage {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);
};

class StarSystemLegacyGeneratorBase : public StarSystemGeneratorStage {
protected:
	struct StarTypeInfo {
		SystemBody::BodySuperType supertype;
		int mass[2]; // min,max % sol for stars, unused for planets
		int radius[2]; // min,max % sol radii for stars, % earth radii for planets
		int tempMin, tempMax;
	};
	static const fixed starMetallicities[];
	static const StarTypeInfo starTypeInfo[];

	void PickAtmosphere(SystemBody* sbody);
	void PickRings(SystemBody* sbody, bool forceRings = false);
	fixed CalcHillRadius(SystemBody* sbody) const;
};

class StarSystemCustomGenerator : public StarSystemLegacyGeneratorBase {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void CustomGetKidsOf(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *parent, const std::vector<CustomSystemBody*> &children, int *outHumanInfestedness, Random &rand);
};

class StarSystemRandomGenerator : public StarSystemLegacyGeneratorBase {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void MakePlanetsAround(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *primary, Random &rand);
	void MakeRandomStar(SystemBody *sbody, Random &rand);
	void MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type, Random &rand);
	void MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass, Random &rand);
	void MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist, Random &rand);

	int CalcSurfaceTemp(const SystemBody *primary, fixed distToPrimary, fixed albedo, fixed greenhouse);
	const SystemBody* FindStarAndTrueOrbitalRange(const SystemBody *planet, fixed &orbMin_, fixed &orbMax_) const;
	void PickPlanetType(SystemBody *sbody, Random &rand);
};

class PopulateStarSystemGenerator : public StarSystemLegacyGeneratorBase {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void SetSysPolit(RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, const fixed &human_infestedness);
	void SetCommodityLegality(RefCountedPtr<StarSystem::GeneratorAPI> system);
	void SetEconType(RefCountedPtr<StarSystem::GeneratorAPI> system);

	void PopulateAddStations(SystemBody* sbody, StarSystem::GeneratorAPI* system);
	void PositionSettlementOnPlanet(SystemBody* sbody, std::vector<double> &prevOrbits);
	void PopulateStage1(SystemBody* sbody, StarSystem::GeneratorAPI* system, fixed &outTotalPop);
};

#endif
