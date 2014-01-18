// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Pi.h"
#include "galaxy/Galaxy.h"
#include "GalaxyGenerator.h"
#include "SectorGenerator.h"
#include "galaxy/StarSystemGenerator.h"

static const GalaxyGenerator::Version LAST_VERSION_LEGACY = 1;

std::string GalaxyGenerator::s_defaultGenerator = "legacy";
GalaxyGenerator::Version GalaxyGenerator::s_defaultVersion = LAST_VERSION_LEGACY;
RefCountedPtr<Galaxy> GalaxyGenerator::s_galaxy;

//static
void GalaxyGenerator::Init(const std::string& name, Version version)
{
	s_defaultGenerator = name;
	s_defaultVersion = (version == LAST_VERSION) ? GetLastVersion(name) : version;
	GalaxyGenerator::Create(); // This will set s_galaxy
}

//static
void GalaxyGenerator::Uninit()
{
	s_galaxy->FlushCaches();
	s_galaxy.Reset();
}

//static
GalaxyGenerator::Version GalaxyGenerator::GetLastVersion(const std::string& name)
{
	if (name == "legacy")
		return LAST_VERSION_LEGACY;
	else
		return LAST_VERSION;
}

// static
RefCountedPtr<Galaxy> GalaxyGenerator::Create(const std::string& name, Version version)
{
	if (version == LAST_VERSION)
		version = GetLastVersion(name);

	RefCountedPtr<GalaxyGenerator> galgen;
	if (name == "legacy") {
		Output("Creating new galaxy generator '%s' version %d\n", name.c_str(), version);
		if (version == 0 || version == 1) {
			galgen.Reset((new GalaxyGenerator(name, version))
				->AddSectorStage(new SectorCustomSystemsGenerator(CustomSystem::CUSTOM_ONLY_RADIUS))
				->AddSectorStage(new SectorRandomSystemsGenerator)
				->AddSectorStage(new SectorPersistenceGenerator(version))
				->AddStarSystemStage(new StarSystemFromSectorGenerator)
				->AddStarSystemStage(new StarSystemCustomGenerator)
				->AddStarSystemStage(new StarSystemRandomGenerator)
				->AddStarSystemStage(new PopulateStarSystemGenerator));
		}
	}

	if (galgen) {
		if (s_galaxy && galgen->m_name == s_galaxy->GetGeneratorName() && galgen->m_version == s_galaxy->GetGeneratorVersion()) {
			Output("Clearing and re-using previous Galaxy object\n");
			s_galaxy->SetGalaxyGenerator(galgen);
	        s_galaxy->FlushCaches();
	        return s_galaxy;
		}

		assert(name == "legacy"); // Once whe have have more, this will become an if switch
		// NB : The galaxy density image MUST be in BMP format due to OSX failing to load pngs the same as Linux/Windows
		s_galaxy = RefCountedPtr<Galaxy>(new DensityMapGalaxy(galgen, "galaxy_dense.bmp", 50000.0, 25000.0, 0.0, "factions", "systems"));
		s_galaxy->Init();
		return s_galaxy;
	} else {
		Output("Galaxy generation failed: Unknown generator '%s' version %d\n", name.c_str(), version);
		return RefCountedPtr<Galaxy>();
	}
}

// static
RefCountedPtr<Galaxy> GalaxyGenerator::Create(Serializer::Reader& rd)
{
	std::string genName = rd.String();
	GalaxyGenerator::Version genVersion = rd.Int32();
	RefCountedPtr<Galaxy> galaxy = GalaxyGenerator::Create(genName, genVersion);
	if (!galaxy) {
		Output("can't load savefile, unsupported galaxy generator %s, version %d\n", genName.c_str(), genVersion);
		throw SavedGameWrongVersionException();
	}
	return galaxy;
}

void GalaxyGenerator::Serialize(Serializer::Writer &wr, RefCountedPtr<Galaxy> galaxy)
{
	wr.String(m_name);
	if (m_name == "legacy" && m_version == 0)
		wr.Int32(1); // Promote savegame
	else
		wr.Int32(m_version);
	for (SectorGeneratorStage* secgen : m_sectorStage)
		secgen->Serialize(wr, galaxy);
	for (StarSystemGeneratorStage* sysgen : m_starSystemStage)
		sysgen->Serialize(wr, galaxy);
}

void GalaxyGenerator::Unserialize(Serializer::Reader &rd, RefCountedPtr<Galaxy> galaxy)
{
	for (SectorGeneratorStage* secgen : m_sectorStage)
		secgen->Unserialize(rd, galaxy);
	for (StarSystemGeneratorStage* sysgen : m_starSystemStage)
		sysgen->Unserialize(rd, galaxy);
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
	RefCountedPtr<Sector> sector(new Sector(galaxy, path, cache));
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
