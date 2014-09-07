// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef STARSYSTEM_GENERATOR_H
#define STARSYSTEM_GENERATOR_H

#include "StarSystem.h"
#include "GalaxyGenerator.h"

class StarSystemFromSectorGenerator : public StarSystemGeneratorStage {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);
};

class StarSystemCustomGenerator : public StarSystemGeneratorStage {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void CustomGetKidsOf(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *parent, const std::vector<CustomSystemBody*> &children, int *outHumanInfestedness, Random &rand);
};

class StarSystemRandomGenerator : public StarSystemGeneratorStage {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void MakePlanetsAround(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *primary, Random &rand);
	void MakeRandomStar(SystemBody *sbody, Random &rand);
	void MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type, Random &rand);
	void MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass, Random &rand);
	void MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist, Random &rand);
};

class PopulateStarSystemGenerator : public StarSystemGeneratorStage {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void MakeShortDescription(RefCountedPtr<StarSystem::GeneratorAPI> system, Random &rand);
};

#endif
