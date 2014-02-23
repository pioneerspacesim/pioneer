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

//#define DEBUG_SECTOR_CACHE

void SectorCache::AddToCache(std::vector<RefCountedPtr<Sector> >& sec)
{
	for (auto it = sec.begin(), itEnd = sec.end(); it != itEnd; ++it) {
		auto inserted = m_sectorAttic.insert( std::make_pair(it->Get()->GetSystemPath(), it->Get()) );
		if (!inserted.second) {
			it->Reset(inserted.first->second);
		}
	}
}

RefCountedPtr<Sector> SectorCache::GetIfCached(const SystemPath& loc)
{
	PROFILE_SCOPED()
	SystemPath secPath = loc.SectorOnly();

	RefCountedPtr<Sector> s;
	SectorAtticMap::iterator i = m_sectorAttic.find(secPath);
	if (i != m_sectorAttic.end()) {
		s.Reset(i->second);
	}

	return s;
}

RefCountedPtr<Sector> SectorCache::GetCached(const SystemPath& loc)
{
	PROFILE_SCOPED()
	SystemPath secPath = loc.SectorOnly();

	RefCountedPtr<Sector> s = GetIfCached(secPath);
	if (!s) {
		s.Reset(new Sector(secPath));
		m_sectorAttic.insert( std::make_pair(secPath, s.Get()));
		if (Faction::MayAssignFactions())
			s->AssignFactions();
		else
			m_unassignedFactionsSet.insert(s.Get());
	}

	return s;
}

bool SectorCache::HasCached(const SystemPath& loc) const
{
	PROFILE_SCOPED()

	assert(loc.IsSectorPath());
	return (m_sectorAttic.find(loc) != m_sectorAttic.end());
}

void SectorCache::RemoveFromAttic(const SystemPath& path)
{
	auto it = m_sectorAttic.find(path);
	if (it != m_sectorAttic.end()) {
		m_unassignedFactionsSet.erase(it->second);
		m_sectorAttic.erase(it);
	}
}

void SectorCache::ClearCache()
{
	for (auto it = m_slaves.begin(), itEnd = m_slaves.end(); it != itEnd; ++it)
		(*it)->ClearCache();
}

void SectorCache::AssignFactions()
{
	assert(Faction::MayAssignFactions());
	for (Sector* s : m_unassignedFactionsSet) {
		s->AssignFactions();
	}
	m_unassignedFactionsSet.clear();
}

RefCountedPtr<SectorCache::Slave> SectorCache::NewSlaveCache()
{
	return RefCountedPtr<Slave>(new Slave);
}

SectorCache::Slave::Slave() : m_jobs(Pi::Jobs())
{
	Sector::cache.m_slaves.insert(this);
}

RefCountedPtr<Sector> SectorCache::Slave::GetIfCached(const SystemPath& loc)
{
	PROFILE_SCOPED()
	SystemPath secPath = loc.SectorOnly();

	SectorCacheMap::iterator i = m_sectorCache.find(secPath);
	if (i != m_sectorCache.end())
		return (*i).second;
	return RefCountedPtr<Sector>();
}

RefCountedPtr<Sector> SectorCache::Slave::GetCached(const SystemPath& loc)
{
	PROFILE_SCOPED()
	SystemPath secPath = loc.SectorOnly();

	SectorCacheMap::iterator i = m_sectorCache.find(secPath);
	if (i != m_sectorCache.end())
		return (*i).second;

	auto inserted = m_sectorCache.insert( std::make_pair(secPath, Sector::cache.GetCached(secPath)) );
	return inserted.first->second;
}

void SectorCache::Slave::Erase(const SystemPath& loc) { m_sectorCache.erase(loc); }
void SectorCache::Slave::Erase(const SectorCacheMap::const_iterator& it) { m_sectorCache.erase(it); }
void SectorCache::Slave::ClearCache() { m_sectorCache.clear(); }

SectorCache::Slave::~Slave()
{
#	ifdef DEBUG_SECTOR_CACHE
		unsigned unique = 0;
		for (auto it = m_sectorCache.begin(); it != m_sectorCache.end(); ++it)
			if (it->second->GetRefCount() == 1)
				unique++;
		Output("SectorCache: Discarding slave cache with %zu entries (%u to be removed)\n", m_sectorCache.size(), unique);
#	endif
	Sector::cache.m_slaves.erase(this);
}

//void SectorCache::Slave::Insert(RefCountedPtr<Sector> sec)
//{
//	m_sectorCache.insert( std::make_pair(sec->GetSystemPath(), sec) );
//}

void SectorCache::Slave::AddToCache(const std::vector<RefCountedPtr<Sector> >& secIn)
{
	for (auto it = secIn.begin(), itEnd = secIn.end(); it != itEnd; ++it) {
		m_sectorCache.insert( std::make_pair(it->Get()->GetSystemPath(), *it) );
	}
}

void SectorCache::Slave::FillCache(const SectorCache::PathVector& paths)
{
	// allocate some space for what we're about to chunk up
	std::vector<std::unique_ptr<PathVector> > vec_paths;
	vec_paths.reserve(paths.size()/CACHE_JOB_SIZE + 1);
	std::unique_ptr<PathVector> current_paths;
#	ifdef DEBUG_SECTOR_CACHE
		size_t alreadyCached = m_sectorCache.size();
		unsigned masterCached = 0;
		unsigned toBeCreated = 0;
#	endif

	// chop the paths into groups of CACHE_JOB_SIZE
	for (auto it = paths.begin(), itEnd = paths.end(); it != itEnd; ++it) {
		RefCountedPtr<Sector> s = Sector::cache.GetIfCached(*it);
		if (s) {
			m_sectorCache[*it] = s;
#			ifdef DEBUG_SECTOR_CACHE
				++masterCached;
#			endif
		} else {
			if (!current_paths) {
				current_paths.reset(new PathVector);
				current_paths->reserve(CACHE_JOB_SIZE);
			}
			current_paths->push_back(*it);
			if( current_paths->size() >= CACHE_JOB_SIZE ) {
				vec_paths.push_back( std::move(current_paths) );
			}
#			ifdef DEBUG_SECTOR_CACHE
				++toBeCreated;
#			endif
		}
	}

	// catch the last loop in case it's got some entries (could be less than the spread width)
	if (current_paths) {
		vec_paths.push_back( std::move(current_paths) );
	}

#	ifdef DEBUG_SECTOR_CACHE
		Output("SectorCache: FillCache: %zu cached, %u in master cache, %u to be created, will use %zu jobs\n",  alreadyCached, masterCached, toBeCreated, vec_paths.size());
#	endif

	// now add the batched jobs
	for (auto it = vec_paths.begin(), itEnd = vec_paths.end(); it != itEnd; ++it)
		m_jobs.Order(new SectorCacheJob(std::move(*it), this));
}


SectorCache::SectorCacheJob::SectorCacheJob(std::unique_ptr<std::vector<SystemPath> > path, SectorCache::Slave* slaveCache)
	: Job(), m_paths(std::move(path)), m_slaveCache(slaveCache)
{
	m_sectors.reserve(m_paths->size());
	assert(Faction::MayAssignFactions());
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
	Sector::cache.AddToCache(m_sectors); // This modifies the vector to the sectors already in the master cache
	m_slaveCache->AddToCache(m_sectors);
}
