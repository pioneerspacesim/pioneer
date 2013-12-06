// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Factions.h"
#include "Pi.h"
#include "SectorCache.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"

using namespace Graphics;

static const int DRAW_RAD = 5;
static const float FAR_THRESHOLD = 7.5f;

SectorCache::SectorCacheMap SectorCache::m_sectorCache;
int SectorCache::m_cacheXMin = 0;
int SectorCache::m_cacheXMax = 0;
int SectorCache::m_cacheYMin = 0;
int SectorCache::m_cacheYMax = 0;
int SectorCache::m_cacheZMin = 0;
int SectorCache::m_cacheZMax = 0;
float SectorCache::m_zoomClamped = 1.0f;
vector3f SectorCache::m_pos;

void SectorCache::Init()
{
}

void SectorCache::DeInit()
{
	SectorCacheMap::iterator iter = m_sectorCache.begin();
	while (iter != m_sectorCache.end())	{
		Sector *s = (*iter).second;
		if (s) {
			delete s;
		}
		iter++;
	}
	m_sectorCache.clear();
}

Sector* SectorCache::GetCached(const SystemPath& loc)
{
	PROFILE_SCOPED()
	Sector *s = 0;

	SectorCacheMap::iterator i = m_sectorCache.find(loc);
	if (i != m_sectorCache.end())
		return (*i).second;

	s = new Sector(loc.sectorX, loc.sectorY, loc.sectorZ);
	m_sectorCache.insert( std::pair<SystemPath,Sector*>(loc, s) );
	s->AssignFactions();

	return s;
}

Sector* SectorCache::GetCached(const int sectorX, const int sectorY, const int sectorZ)
{
	PROFILE_SCOPED()
	const SystemPath loc(sectorX, sectorY, sectorZ);
	return GetCached(loc);
}

void SectorCache::ShrinkCache()
{
	PROFILE_SCOPED()
	// we're going to use these to determine if our sectors are within the range that we'll ever render
	const int drawRadius = (m_zoomClamped <= FAR_THRESHOLD) ? DRAW_RAD : ceilf((m_zoomClamped/FAR_THRESHOLD) * DRAW_RAD);

	const int xmin = int(floorf(m_pos.x))-drawRadius;
	const int xmax = int(floorf(m_pos.x))+drawRadius;
	const int ymin = int(floorf(m_pos.y))-drawRadius;
	const int ymax = int(floorf(m_pos.y))+drawRadius;
	const int zmin = int(floorf(m_pos.z))-drawRadius;
	const int zmax = int(floorf(m_pos.z))+drawRadius;

	// XXX don't clear the current/selected/target sectors

	if  (xmin != m_cacheXMin || xmax != m_cacheXMax
	  || ymin != m_cacheYMin || ymax != m_cacheYMax
	  || zmin != m_cacheZMin || zmax != m_cacheZMax) {
		SectorCacheMap::iterator iter = m_sectorCache.begin();
		while (iter != m_sectorCache.end())	{
			Sector *s = (*iter).second;
			//check_point_in_box
			if (s && !s->WithinBox( xmin, xmax, ymin, ymax, zmin, zmax )) {
				delete s;
				m_sectorCache.erase( iter++ );
			} else {
				iter++;
			}
		}

		m_cacheXMin = xmin;
		m_cacheXMax = xmax;
		m_cacheYMin = ymin;
		m_cacheYMax = ymax;
		m_cacheZMin = zmin;
		m_cacheZMax = zmax;
	}
}
