// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SECTORGENERATOR_H
#define SECTORGENERATOR_H

#include "Random.h"
#include "RefCounted.h"
#include "Sector.h"
#include "GalaxyGenerator.h"
#include "PersistSystemData.h"
#include "StarSystem.h"

class SectorCustomSystemsGenerator : public SectorGeneratorStage {
public:
	SectorCustomSystemsGenerator(int customOnlyRadius) : m_customOnlyRadius(customOnlyRadius) { }
	virtual bool Apply(Random& rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig* config);

private:
	int m_customOnlyRadius;
};

class SectorRandomSystemsGenerator : public SectorGeneratorStage {
public:
	virtual bool Apply(Random& rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig* config);
private:
	const std::string GenName(RefCountedPtr<Galaxy> galaxy, const Sector& sec, Sector::System &sys, int si, Random &rand);
};

class SectorPersistenceGenerator : public SectorGeneratorStage {
public:
	SectorPersistenceGenerator(GalaxyGenerator::Version version) : m_version(version) { }
	virtual bool Apply(Random& rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig* config);
	virtual void Unserialize(Serializer::Reader &rd, RefCountedPtr<Galaxy> galaxy);
	virtual void Serialize(Serializer::Writer &wr, RefCountedPtr<Galaxy> galaxy);

private:
	void SetExplored(Sector::System* sys, StarSystem::ExplorationState e, double time);

	const GalaxyGenerator::Version m_version;
	PersistSystemData<Sint32> m_exploredSystems;
};

#endif
