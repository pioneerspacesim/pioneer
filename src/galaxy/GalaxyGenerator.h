// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GALAXYGENERATOR_H
#define GALAXYGENERATOR_H

#include "RefCounted.h"
#include "Sector.h"
#include "StarSystem.h"
#include "SystemPath.h"
#include <list>
#include <string>

class SectorGeneratorStage;
class StarSystemGeneratorStage;

class GalaxyGenerator : public RefCounted {
public:
	typedef int Version;
	static const Version LAST_VERSION = -1;

	static void Init(const std::string &name = std::string("legacy"), Version version = LAST_VERSION);
	static void Uninit();

	static RefCountedPtr<Galaxy> Create(const std::string &name, Version version = LAST_VERSION);
	static RefCountedPtr<Galaxy> Create()
	{
		return Create(s_defaultGenerator, s_defaultVersion);
	}
	static RefCountedPtr<Galaxy> CreateFromJson(const Json &jsonObj);

	static std::string GetDefaultGeneratorName() { return s_defaultGenerator; }
	static Version GetDefaultGeneratorVersion() { return s_defaultVersion; }
	static Version GetLastVersion(const std::string &name);

	virtual ~GalaxyGenerator();

	const std::string &GetName() const { return m_name; }
	Version GetVersion() const { return m_version; }

	bool IsDefault() const { return m_name == s_defaultGenerator && m_version == s_defaultVersion; }

	void ToJson(Json &jsonObj, RefCountedPtr<Galaxy> galaxy);
	void FromJson(const Json &jsonObj, RefCountedPtr<Galaxy> galaxy);

	// Templated for the template cache class.
	template <typename T, typename Cache>
	RefCountedPtr<T> Generate(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, Cache *cache);

	GalaxyGenerator *AddSectorStage(SectorGeneratorStage *sectorGenerator);
	GalaxyGenerator *AddStarSystemStage(StarSystemGeneratorStage *starSystemGenerator);

	struct SectorConfig {
		bool isCustomOnly;

		SectorConfig() :
			isCustomOnly(false) {}
	};

	struct StarSystemConfig {
		bool isCustomOnly;

		StarSystemConfig() :
			isCustomOnly(false) {}
	};

private:
	GalaxyGenerator(const std::string &name, Version version = LAST_VERSION) :
		m_name(name),
		m_version(version) {}

	virtual RefCountedPtr<Sector> GenerateSector(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, SectorCache *cache);
	virtual RefCountedPtr<StarSystem> GenerateStarSystem(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, StarSystemCache *cache);

	const std::string m_name;
	const Version m_version;

	std::list<SectorGeneratorStage *> m_sectorStage;
	std::list<StarSystemGeneratorStage *> m_starSystemStage;

	static RefCountedPtr<Galaxy> s_galaxy;
	static std::string s_defaultGenerator;
	static Version s_defaultVersion;
};

template <>
inline RefCountedPtr<Sector> GalaxyGenerator::Generate<Sector, SectorCache>(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, SectorCache *cache)
{
	return GenerateSector(galaxy, path, cache);
}

template <>
inline RefCountedPtr<StarSystem> GalaxyGenerator::Generate<StarSystem, StarSystemCache>(RefCountedPtr<Galaxy> galaxy, const SystemPath &path, StarSystemCache *cache)
{
	return GenerateStarSystem(galaxy, path, cache);
}

class GalaxyGeneratorStage {
public:
	virtual ~GalaxyGeneratorStage() {}

	virtual void ToJson(Json &jsonObj, RefCountedPtr<Galaxy> galaxy) {}
	virtual void FromJson(const Json &jsonObj, RefCountedPtr<Galaxy> galaxy) {}

protected:
	GalaxyGeneratorStage() :
		m_galaxyGenerator(nullptr) {}

	friend class GalaxyGenerator;
	void AssignToGalaxyGenerator(GalaxyGenerator *galaxyGenerator) { m_galaxyGenerator = galaxyGenerator; }

	GalaxyGenerator *m_galaxyGenerator;
};

class SectorGeneratorStage : public GalaxyGeneratorStage {
public:
	virtual ~SectorGeneratorStage() {}

	virtual bool Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<Sector> sector, GalaxyGenerator::SectorConfig *config) = 0;
};

class StarSystemGeneratorStage : public GalaxyGeneratorStage {
public:
	virtual ~StarSystemGeneratorStage() {}

	virtual bool Apply(Random &rng, RefCountedPtr<Galaxy> galaxy, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig *config) = 0;
};

#endif
