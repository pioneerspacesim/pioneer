// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <utility>
#include "libs.h"
#include "Factions.h"
#include "Pi.h"
#include "Game.h"
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

void SectorCache::AddToCache(const std::vector<RefCountedPtr<Sector> >& secIn)
{
	for (auto it = secIn.begin(), itEnd = secIn.end(); it != itEnd; ++it) {
		m_sectorCache.insert( std::make_pair(it->Get()->GetSystemPath(), *it) );
	}
}

RefCountedPtr<Sector> SectorCache::GetCached(const SystemPath& loc)
{
	PROFILE_SCOPED()
	SystemPath secPath = loc.SectorOnly();

	SectorCacheMap::iterator i = m_sectorCache.find(secPath);
	if (i != m_sectorCache.end())
		return (*i).second;

	RefCountedPtr<Sector> s;
	SectorAtticMap::iterator j = m_sectorAttic.find(secPath);
	if (j != m_sectorAttic.end()) {
		s = RefCountedPtr<Sector>(j->second);
		m_sectorAttic.erase(j);
	} else {
		s = RefCountedPtr<Sector>(new Sector(secPath));
	}
	m_sectorCache.insert( std::make_pair(secPath, s) );
	s->AssignFactions();

	return s;
}

bool SectorCache::HasCached(const SystemPath& loc) const
{
	PROFILE_SCOPED()

	assert(loc.IsSectorPath());
	const SectorCacheMap::const_iterator i = m_sectorCache.find(loc);
	if (i != m_sectorCache.end())
		return true;
	const SectorAtticMap::const_iterator j = m_sectorAttic.find(loc);
	if (j != m_sectorAttic.end())
		return true;
	return false;
}

bool SectorCache::HasCached(const SystemPath& loc, bool revive)
{
	PROFILE_SCOPED()

	assert(loc.IsSectorPath());
	const SectorCacheMap::const_iterator i = m_sectorCache.find(loc);
	if (i != m_sectorCache.end())
		return true;
	const SectorAtticMap::const_iterator j = m_sectorAttic.find(loc);
	if (j != m_sectorAttic.end()) {
		if (revive) {
			m_sectorCache.insert(std::make_pair(loc, RefCountedPtr<Sector>(j->second)));
			m_sectorAttic.erase(j);
		}
		return true;
	}
	return false;
}

void SectorCache::GenSectorCache()
{
	PROFILE_SCOPED()

	if (!Pi::game || !Pi::game->GetSpace() || !Pi::game->GetSpace()->GetStarSystem().Valid())
		return;

	// current location
	const SystemPath& here = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	const int here_x = here.sectorX;
	const int here_y = here.sectorY;
	const int here_z = here.sectorZ;

	// used to define a cube centred on your current location
	const int diff_sec = 10;
	const int sec_spread = (diff_sec*2)+1; // including the current sector you're in

	typedef std::vector<SystemPath> TVecPaths;
	TVecPaths paths;
	// build all of the possible paths we'll need to build sectors for
	for (int x = here_x-diff_sec; x <= here_x+diff_sec; x++) {
		for (int y = here_y-diff_sec; y <= here_y+diff_sec; y++) {
			for (int z = here_z-diff_sec; z <= here_z+diff_sec; z++) {
				SystemPath path(x, y, z);
				// ignore sectors we've already cached
				if(!HasCached(path, true)) {
					paths.push_back(path);
				}
			}
		}
	}

	// allocate some space for what we're about to chunk up
	std::vector<std::unique_ptr<TVecPaths> > vec_paths;
	vec_paths.reserve(sec_spread * sec_spread);
	std::unique_ptr<TVecPaths> current_paths;

	// chop the paths into groups equivalent to a spread width of the cube
	for (auto it = paths.begin(), itEnd = paths.end(); it != itEnd; ++it) {
		if (!current_paths) {
			current_paths.reset(new TVecPaths);
			current_paths->reserve(sec_spread);
		}
		current_paths->push_back(*it);
		if( current_paths->size() >= sec_spread ) {
			vec_paths.push_back( std::move(current_paths) );
		}
	}
	// catch the last loop in case it's got some entries (could be less than the spread width)
	if(current_paths) {
		vec_paths.push_back( std::move(current_paths) );
	}

	// now add the batched jobs
	for (auto it = vec_paths.begin(), itEnd = vec_paths.end(); it != itEnd; ++it) {
		Pi::Jobs()->Queue(new SectorCacheJob(std::move(*it)));
	}
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
			RefCountedPtr<Sector> s = (*iter).second;
			//check_point_in_box
			if (!s->WithinBox( xmin, xmax, ymin, ymax, zmin, zmax )) {
				m_sectorCache.erase( iter++ );
				m_sectorAttic[SystemPath(s->sx, s->sy, s->sz)] = s.Get();
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

void SectorCache::RemoveFromAttic(const SystemPath& path)
{
	m_sectorAttic.erase(path);
}

SectorCache::SectorCacheJob::SectorCacheJob(std::unique_ptr<std::vector<SystemPath> > path) : Job(), m_paths(std::move(path))
{
	m_sectors.reserve(m_paths->size());
}

//virtual
void SectorCache::SectorCacheJob::OnRun()    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
{
	for (auto it = m_paths->begin(), itEnd = m_paths->end(); it != itEnd; ++it) {
		RefCountedPtr<Sector> newSec(new Sector(*it));
		newSec->AssignFactions();
		m_sectors.push_back( newSec );
	}
}
//virtual
void SectorCache::SectorCacheJob::OnFinish()  // runs in primary thread of the context
{
	Sector::cache.AddToCache( m_sectors );
}
