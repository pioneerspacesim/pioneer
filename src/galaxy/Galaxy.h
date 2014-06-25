// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GALAXY_H
#define _GALAXY_H

#include <cstdio>
#include "Factions.h"
#include "CustomSystem.h"
#include "GalaxyCache.h"

struct SDL_Surface;

class Galaxy {
public:
	// lightyears
	const float GALAXY_RADIUS;
	const float SOL_OFFSET_X;
	const float SOL_OFFSET_Y;

	Galaxy();
	~Galaxy();

	void Init();

	SDL_Surface *GetGalaxyBitmap();
	/* 0 - 255 */
	Uint8 GetSectorDensity(int sx, int sy, int sz);
	FactionsDatabase* GetFactions() { return &m_factions; } // XXX const correctness
	CustomSystemsDatabase* GetCustomSystems() { return &m_customSystems; } // XXX const correctness

	RefCountedPtr<const Sector> GetSector(const SystemPath& path) { return m_sectorCache.GetCached(path); }
	RefCountedPtr<SectorCache::Slave> NewSectorSlaveCache() { return m_sectorCache.NewSlaveCache(); }

	RefCountedPtr<StarSystem> GetStarSystem(const SystemPath& path) { return m_starSystemCache->GetCached(path); }
	RefCountedPtr<StarSystemCache::Slave> GetStarSystemCache() { return m_starSystemCache; }
	RefCountedPtr<StarSystemCache::Slave> NewStarSystemSlaveCache() { return m_starSystemAttic.NewSlaveCache(); }

	void FlushCaches();
	void Dump(FILE* file, Sint32 centerX, Sint32 centerY, Sint32 centerZ, Sint32 radius);

private:
	SDL_Surface *m_galaxybmp;
	SectorCache m_sectorCache;
	StarSystemCache m_starSystemAttic;
	RefCountedPtr<StarSystemCache::Slave> m_starSystemCache;
	FactionsDatabase m_factions;
	CustomSystemsDatabase m_customSystems;
};

#endif /* _GALAXY_H */
