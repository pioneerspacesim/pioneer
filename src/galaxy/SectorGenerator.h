// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SECTORGENERATOR_H
#define SECTORGENERATOR_H

#include "Random.h"
#include "RefCounted.h"
#include "Sector.h"
#include "GalaxyGenerator.h"

class SectorCustomSystemsGenerator : public SectorGeneratorStage {
public:
	SectorCustomSystemsGenerator(int customOnlyRadius) : m_customOnlyRadius(customOnlyRadius) { }
	virtual bool Apply(Random& rng, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig* config);

private:
	int m_customOnlyRadius;
};

class SectorRandomSystemsGenerator : public SectorGeneratorStage {
public:
virtual bool Apply(Random& rng, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig* config);
private:
	const std::string GenName(const Sector& sec, Sector::System &sys, int si, Random &rand);
};

#endif
