// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GALAXY_H
#define _GALAXY_H

#include <cstdio>
#include "RefCounted.h"
#include "Factions.h"
#include "CustomSystem.h"
#include "GalaxyCache.h"

struct SDL_Surface;
class GalaxyGenerator;

class Galaxy : public RefCounted {
private:
	friend class GalaxyGenerator;
	Galaxy(RefCountedPtr<GalaxyGenerator> galaxyGenerator);

public:
	// lightyears
	const float GALAXY_RADIUS;
	const float SOL_OFFSET_X;
	const float SOL_OFFSET_Y;

	~Galaxy();

	void Init();

	/* 0 - 255 */
	Uint8 GetSectorDensity(const int sx, const int sy, const int sz) const;
	FactionsDatabase* GetFactions() { return &m_factions; } // XXX const correctness
	CustomSystemsDatabase* GetCustomSystems() { return &m_customSystems; } // XXX const correctness

	RefCountedPtr<const Sector> GetSector(const SystemPath& path) { return m_sectorCache.GetCached(path); }
	RefCountedPtr<SectorCache::Slave> NewSectorSlaveCache() { return m_sectorCache.NewSlaveCache(); }

	RefCountedPtr<StarSystem> GetStarSystem(const SystemPath& path) { return m_starSystemCache.GetCached(path); }
	RefCountedPtr<StarSystemCache::Slave> NewStarSystemSlaveCache() { return m_starSystemCache.NewSlaveCache(); }

	void FlushCaches();
	void Dump(FILE* file, Sint32 centerX, Sint32 centerY, Sint32 centerZ, Sint32 radius);

	const GalaxyGenerator& GetGenerator() const { return *m_galaxyGenerator.Get(); }
	const std::string& GetGeneratorName() const;
	int GetGeneratorVersion() const;

private:
	RefCountedPtr<GalaxyGenerator> m_galaxyGenerator;
	std::unique_ptr<float[]> m_galaxyMap;
	Sint32 m_mapWidth, m_mapHeight;
	SectorCache m_sectorCache;
	StarSystemCache m_starSystemCache;
	FactionsDatabase m_factions;
	CustomSystemsDatabase m_customSystems;
};

#endif /* _GALAXY_H */
