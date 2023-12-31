// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GalaxyGenerator.h"

#include "GameSaveError.h"
#include "Json.h"
#include "SectorGenerator.h"
#include "galaxy/Galaxy.h"
#include "galaxy/StarSystemGenerator.h"
#include "utils.h"

static const GalaxyGenerator::Version LAST_VERSION_LEGACY = 1;

std::string GalaxyGenerator::s_defaultGenerator = "legacy";
GalaxyGenerator::Version GalaxyGenerator::s_defaultVersion = LAST_VERSION_LEGACY;
RefCountedPtr<Galaxy> GalaxyGenerator::s_galaxy;

//static
void GalaxyGenerator::Init(const std::string &name, Version version)
{
	PROFILE_SCOPED()
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
GalaxyGenerator::Version GalaxyGenerator::GetLastVersion(const std::string &name)
{
	if (name == "legacy")
		return LAST_VERSION_LEGACY;
	else
		return LAST_VERSION;
}

// static
RefCountedPtr<Galaxy> GalaxyGenerator::Create(const std::string &name, Version version)
{
	PROFILE_SCOPED()
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
RefCountedPtr<Galaxy> GalaxyGenerator::CreateFromJson(const Json &jsonObj)
{
	try {
		std::string genName = jsonObj["name"];
		GalaxyGenerator::Version genVersion = jsonObj["version"];

		RefCountedPtr<Galaxy> galaxy = GalaxyGenerator::Create(genName, genVersion);
		if (!galaxy) {
			Output("can't load savefile, unsupported galaxy generator %s, version %d\n", genName.c_str(), genVersion);
			throw SavedGameWrongVersionException();
		}
		return galaxy;
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void GalaxyGenerator::ToJson(Json &jsonObj, RefCountedPtr<Galaxy> galaxy)
{
	Json galaxyGenObj({}); // Create JSON object to contain galaxy data.

	galaxyGenObj["name"] = m_name;

	if (m_name == "legacy" && m_version == 0)
		galaxyGenObj["version"] = 1; // Promote savegame
	else
		galaxyGenObj["version"] = m_version;

	Json sectorStageArray = Json::array(); // Create JSON array to contain sector stage data.
	for (SectorGeneratorStage *secgen : m_sectorStage) {
		Json sectorStageArrayEl({}); // Create JSON object to contain sector stage element.
		secgen->ToJson(sectorStageArrayEl, galaxy);
		sectorStageArray.push_back(sectorStageArrayEl); // Append sector stage object to array.
	}
	Json starSystemStageArray = Json::array(); // Create JSON array to contain system stage data.
	for (StarSystemGeneratorStage *sysgen : m_starSystemStage) {
		Json starSystemStageArrayEl({}); // Create JSON object to contain system stage element.
		sysgen->ToJson(starSystemStageArrayEl, galaxy);
		starSystemStageArray.push_back(starSystemStageArrayEl); // Append system stage object to array.
	}
	galaxyGenObj["sector_stage"] = sectorStageArray;		  // Add sector stage array to galaxy generator object.
	galaxyGenObj["star_system_stage"] = starSystemStageArray; // Add system stage array to galaxy generator object.

	jsonObj["galaxy_generator"] = galaxyGenObj; // Add galaxy generator object to supplied object.
}

void GalaxyGenerator::FromJson(const Json &jsonObj, RefCountedPtr<Galaxy> galaxy)
{
	try {
		Json sectorStageArray = jsonObj["sector_stage"].get<Json::array_t>();
		Json starSystemStageArray = jsonObj["star_system_stage"].get<Json::array_t>();

		unsigned int arrayIndex = 0;
		for (SectorGeneratorStage *secgen : m_sectorStage)
			secgen->FromJson(sectorStageArray[arrayIndex++], galaxy);
		arrayIndex = 0;
		for (StarSystemGeneratorStage *sysgen : m_starSystemStage)
			sysgen->FromJson(starSystemStageArray[arrayIndex++], galaxy);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

GalaxyGenerator::~GalaxyGenerator()
{
	for (SectorGeneratorStage *secgen : m_sectorStage)
		delete secgen;
	for (StarSystemGeneratorStage *sysgen : m_starSystemStage)
		delete sysgen;
}

GalaxyGenerator *GalaxyGenerator::AddSectorStage(SectorGeneratorStage *sectorGenerator)
{
	auto it = m_sectorStage.insert(m_sectorStage.end(), sectorGenerator);
	(*it)->AssignToGalaxyGenerator(this);
	return this;
}

GalaxyGenerator *GalaxyGenerator::AddStarSystemStage(StarSystemGeneratorStage *starSystemGenerator)
{
	auto it = m_starSystemStage.insert(m_starSystemStage.end(), starSystemGenerator);
	(*it)->AssignToGalaxyGenerator(this);
	return this;
}

RefCountedPtr<Sector> GalaxyGenerator::GenerateSector(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, SectorCache *cache)
{
	const Uint32 _init[4] = { Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rng(_init, 4);
	SectorConfig config;
	RefCountedPtr<Sector> sector(new Sector(galaxy, path, cache));
	for (SectorGeneratorStage *secgen : m_sectorStage)
		if (!secgen->Apply(rng, galaxy, sector, &config))
			break;
	return sector;
}

RefCountedPtr<StarSystem> GalaxyGenerator::GenerateStarSystem(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, StarSystemCache *cache)
{
	PROFILE_SCOPED()
	RefCountedPtr<const Sector> sec = galaxy->GetSector(path);
	assert(path.systemIndex < sec->m_systems.size());
	Uint32 seed = sec->m_systems[path.systemIndex].GetSeed();
	std::string name = sec->m_systems[path.systemIndex].GetName();
	Uint32 _init[5] = { Uint32(seed), Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rng(_init, 5);
	StarSystemConfig config;
	RefCountedPtr<StarSystem::GeneratorAPI> system(new StarSystem::GeneratorAPI(path, galaxy, cache, rng));
	for (StarSystemGeneratorStage *sysgen : m_starSystemStage)
		if (!sysgen->Apply(rng, galaxy, system, &config))
			break;
	return system;
}
