// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "galaxy/Galaxy.h"
#include "GalaxyGenerator.h"
#include "SectorGenerator.h"
#include "galaxy/StarSystemGenerator.h"

static const GalaxyGenerator::Version LAST_VERSION_LEGACY = 0;

std::string GalaxyGenerator::s_defaultGenerator = "legacy";
GalaxyGenerator::Version GalaxyGenerator::s_defaultVersion = LAST_VERSION_LEGACY;

//static
GalaxyGenerator::Version GalaxyGenerator::GetLastVersion(const std::string& name)
{
	if (name == "legacy")
		return LAST_VERSION_LEGACY;
	else
		return LAST_VERSION;
}

//static
void GalaxyGenerator::SetDefaultGenerator(const std::string& name, Version version) {
	s_defaultGenerator = name;
	s_defaultVersion = (version == LAST_VERSION) ? GetLastVersion(name) : version;
}

// static
RefCountedPtr<Galaxy> GalaxyGenerator::Create(const std::string& name, Version version)
{
	if (version == LAST_VERSION)
		version = GetLastVersion(name);
	if (name == "legacy") {
		Output("Creating new galaxy with generator '%s' version %d\n", name.c_str(), version);
		if (version == 0) {
			return RefCountedPtr<Galaxy>(new Galaxy(RefCountedPtr<GalaxyGenerator>(
				(new GalaxyGenerator(name, version))
				->AddSectorStage(new SectorCustomSystemsGenerator(CustomSystem::CUSTOM_ONLY_RADIUS))
				->AddSectorStage(new SectorRandomSystemsGenerator)
				->AddStarSystemStage(new StarSystemFromSectorGenerator)
				->AddStarSystemStage(new StarSystemCustomGenerator)
				->AddStarSystemStage(new StarSystemRandomGenerator)
				->AddStarSystemStage(new PopulateStarSystemGenerator))));
		}
	}
	Output("Galaxy generation failed: Unknown generator '%s' version %d\n", name.c_str(), version);
	return RefCountedPtr<Galaxy>();
}

GalaxyGenerator::~GalaxyGenerator()
{
	for (SectorGeneratorStage* secgen : m_sectorStage)
		delete secgen;
	for (StarSystemGeneratorStage* sysgen : m_starSystemStage)
		delete sysgen;
}

GalaxyGenerator* GalaxyGenerator::AddSectorStage(SectorGeneratorStage* sectorGenerator)
{
	auto it = m_sectorStage.insert(m_sectorStage.end(), sectorGenerator);
	(*it)->AssignToGalaxyGenerator(this);
	return this;
}

GalaxyGenerator* GalaxyGenerator::AddStarSystemStage(StarSystemGeneratorStage* starSystemGenerator)
{
	auto it = m_starSystemStage.insert(m_starSystemStage.end(), starSystemGenerator);
	(*it)->AssignToGalaxyGenerator(this);
	return this;
}

RefCountedPtr<Sector> GalaxyGenerator::GenerateSector(RefCountedPtr<Galaxy> galaxy, const SystemPath& path, SectorCache* cache)
{
	const Uint32 _init[4] = { Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rng(_init, 4);
	SectorConfig config;
	RefCountedPtr<Sector> sector(new Sector(path, cache));
	for (SectorGeneratorStage* secgen : m_sectorStage)
		if (!secgen->Apply(rng, galaxy, sector, &config))
			break;
	return sector;
}

RefCountedPtr<StarSystem> GalaxyGenerator::GenerateStarSystem(RefCountedPtr<Galaxy> galaxy, const SystemPath& path, StarSystemCache* cache)
{
	RefCountedPtr<const Sector> sec = galaxy->GetSector(path);
	assert(path.systemIndex >= 0 && path.systemIndex < sec->m_systems.size());
	Uint32 seed = sec->m_systems[path.systemIndex].GetSeed();
	std::string name = sec->m_systems[path.systemIndex].GetName();
	Uint32 _init[6] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED, Uint32(seed) };
	Random rng(_init, 6);
	StarSystemConfig config;
	RefCountedPtr<StarSystem::GeneratorAPI> system(new StarSystem::GeneratorAPI(path, galaxy, cache, rng));
	for (StarSystemGeneratorStage* sysgen : m_starSystemStage)
		if (!sysgen->Apply(rng, galaxy, system, &config))
			break;
	return system;
}
