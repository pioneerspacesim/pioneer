// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef STARSYSTEM_GENERATOR_H
#define STARSYSTEM_GENERATOR_H

#include "StarSystem.h"
#include "GalaxyGenerator.h"

class StarSystemFromSectorGenerator : public StarSystemGeneratorStage {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<StarSystem> system, GalaxyGenerator::StarSystemConfig* config);
};

class ClassicalStarSystemGenerator : public StarSystemGeneratorStage {
protected:
	void MakeShortDescription(RefCountedPtr<StarSystem> system, Random &rand);
	void Populate(RefCountedPtr<StarSystem> system, bool addSpaceStations);
};

class StarSystemCustomGenerator : public ClassicalStarSystemGenerator {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<StarSystem> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void CustomGetKidsOf(RefCountedPtr<StarSystem> system, SystemBody *parent, const std::vector<CustomSystemBody*> &children, int *outHumanInfestedness, Random &rand);
};

class StarSystemRandomGenerator : public ClassicalStarSystemGenerator {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<StarSystem> system, GalaxyGenerator::StarSystemConfig* config);

private:
	void MakePlanetsAround(RefCountedPtr<StarSystem> system, SystemBody *primary, Random &rand);
	void MakeRandomStar(SystemBody *sbody, Random &rand);
	void MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type, Random &rand);
	void MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass, Random &rand);
	void MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist, Random &rand);
};

#endif
