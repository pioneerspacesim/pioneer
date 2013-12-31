// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

SectorCache::SectorCache() :
	m_cacheXMin(0),
	m_cacheXMax(0),
	m_cacheYMin(0),
	m_cacheYMax(0),
	m_cacheZMin(0),
	m_cacheZMax(0),
	m_zoomClamped(1.0f)
{
}

SectorCache::~SectorCache()
{
	SectorCacheMap::iterator iter = m_sectorCache.begin();
	while (iter != m_sectorCache.end())	{
		Sector *s = (*iter).second;
		delete s;
		iter++;
	}
}

void SectorCache::AddToCache(const std::vector<Sector>& secIn)
{
	for (auto it = secIn.begin(), itEnd = secIn.end(); it != itEnd; ++it) {
		m_sectorCache.insert( std::pair<SystemPath,Sector*>((*it).GetSystemPath(), new Sector((*it))) );
	}
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

bool SectorCache::HasCached(const SystemPath& loc) const
{
	PROFILE_SCOPED()

	const SectorCacheMap::const_iterator i = m_sectorCache.find(loc);
	if (i != m_sectorCache.end())
		return true;

	return false;
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

SectorCacheJob::SectorCacheJob(const std::vector<SystemPath>& path) : Job(), m_paths(path)
{
	m_sectors.reserve(m_paths.size());
}

//virtual
void SectorCacheJob::OnRun()    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
{
	for (auto it = m_paths.begin(), itEnd = m_paths.end(); it != itEnd; ++it) {
		Sector newSec(*it);
		newSec.AssignFactions();
		m_sectors.push_back( newSec );
	}
}
//virtual
void SectorCacheJob::OnFinish()  // runs in primary thread of the context
{
	Sector::cache.AddToCache( m_sectors );
}
