// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include <utility>
#include "libs.h"
#include "Factions.h"
#include "Pi.h"
#include "Game.h"
#include "GalaxyCache.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"

//#define DEBUG_SECTOR_CACHE

//virtual

template <typename T, typename CompareT>
GalaxyObjectCache<T,CompareT>::~GalaxyObjectCache()
{
	for (Slave* s : m_slaves)
		s->MasterDeleted();
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::AddToCache(std::vector<RefCountedPtr<T> >& objects)
{
	for (auto it = objects.begin(), itEnd = objects.end(); it != itEnd; ++it) {
		auto inserted = m_attic.insert( std::make_pair(it->Get()->GetPath(), it->Get()) );
		if (!inserted.second) {
			it->Reset(inserted.first->second);
		} else {
			(*it)->SetCache(this);
		}
	}
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T,CompareT>::GetIfCached(const SystemPath& path)
{
	PROFILE_SCOPED()

	RefCountedPtr<T> s;
	typename AtticMap::iterator i = m_attic.find(path);
	if (i != m_attic.end()) {
		s.Reset(i->second);
	}

	return s;
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T,CompareT>::GetCached(const SystemPath& path)
{
	PROFILE_SCOPED()

	RefCountedPtr<T> s = this->GetIfCached(path);
	if (!s) {
		++m_cacheMisses;
		s.Reset(new T(path, this));
		m_attic.insert( std::make_pair(path, s.Get()));
	} else {
		++m_cacheHits;
	}
	return s;
}

template <typename T, typename CompareT>
bool GalaxyObjectCache<T,CompareT>::HasCached(const SystemPath& path) const
{
	PROFILE_SCOPED()

	return (m_attic.find(path) != m_attic.end());
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::RemoveFromAttic(const SystemPath& path)
{
	m_attic.erase(path);
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::ClearCache()
{
	for (auto it = m_slaves.begin(), itEnd = m_slaves.end(); it != itEnd; ++it)
		(*it)->ClearCache();
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::OutputCacheStatistics(bool reset)
{
	Output("%s: misses: %lld, slave hits: %lld, master hits: %lld\n", CACHE_NAME.c_str(), m_cacheMisses, m_cacheHitsSlave, m_cacheHits);
	if (reset)
		m_cacheMisses = m_cacheHitsSlave = m_cacheHits = 0;
}

template <typename T, typename CompareT>
RefCountedPtr<typename GalaxyObjectCache<T,CompareT>::Slave> GalaxyObjectCache<T,CompareT>::NewSlaveCache()
{
	return RefCountedPtr<Slave>(new Slave(this, Pi::GetAsyncJobQueue()));
}

template <typename T, typename CompareT>
GalaxyObjectCache<T,CompareT>::Slave::Slave(GalaxyObjectCache<T,CompareT>* master, JobQueue* jobQueue)
	: m_master(master), m_jobs(Pi::GetAsyncJobQueue())
{
	m_master->m_slaves.insert(this);
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::Slave::MasterDeleted()
{
	m_master = nullptr;
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T,CompareT>::Slave::GetIfCached(const SystemPath& path)
{
	PROFILE_SCOPED()

	typename CacheMap::iterator i = m_cache.find(path);
	if (i != m_cache.end())
		return (*i).second;
	return RefCountedPtr<T>();
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T,CompareT>::Slave::GetCached(const SystemPath& path)
{
	PROFILE_SCOPED()

	typename CacheMap::iterator i = m_cache.find(path);
	if (i != m_cache.end()) {
		if (m_master)
			++m_master->m_cacheHitsSlave;
		return (*i).second;
	}

	if (m_master) {
		auto inserted = m_cache.insert( std::make_pair(path, m_master->GetCached(path)) );
		return inserted.first->second;
	} else {
		return RefCountedPtr<T>();
	}
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::Slave::Erase(const SystemPath& path) { m_cache.erase(path); }

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::Slave::Erase(const typename CacheMap::const_iterator& it) { m_cache.erase(it); }

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::Slave::ClearCache() { m_cache.clear(); }

template <typename T, typename CompareT>
GalaxyObjectCache<T,CompareT>::Slave::~Slave()
{
#	ifdef DEBUG_SECTOR_CACHE
		unsigned unique = 0;
		for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
			if (it->second->GetRefCount() == 1)
				unique++;
		Output("%s: Discarding slave cache with %zu entries (%u to be removed)\n", CACHE_NAME.c_str(), m_cache.size(), unique);
#	endif
	if (m_master)
		m_master->m_slaves.erase(this);
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::Slave::AddToCache(std::vector<RefCountedPtr<T> >& objects)
{
	if (m_master) {
		m_master->AddToCache(objects); // This modifies the vector to the sectors already in the master cache
		for (auto it = objects.begin(), itEnd = objects.end(); it != itEnd; ++it) {
			m_cache.insert( std::make_pair(it->Get()->GetPath(), *it) );
		}
	}
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::Slave::FillCache(const typename GalaxyObjectCache<T,CompareT>::PathVector& paths,
	typename GalaxyObjectCache<T,CompareT>::CacheFilledCallback callback)
{
	// allocate some space for what we're about to chunk up
	std::vector<std::unique_ptr<PathVector> > vec_paths;
	vec_paths.reserve(paths.size()/CACHE_JOB_SIZE + 1);
	std::unique_ptr<PathVector> current_paths;
#	ifdef DEBUG_SECTOR_CACHE
		size_t alreadyCached = m_cache.size();
		unsigned masterCached = 0;
		unsigned toBeCreated = 0;
#	endif

	// chop the paths into groups of CACHE_JOB_SIZE
	for (auto it = paths.begin(), itEnd = paths.end(); it != itEnd; ++it) {
		RefCountedPtr<T> s = m_master->GetIfCached(*it);
		if (s) {
			m_cache[*it] = s;
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
		Output("%s: FillCache: %zu cached, %u in master cache, %u to be created, will use %zu jobs\n", CACHE_NAME.c_str(),
			alreadyCached, masterCached, toBeCreated, vec_paths.size());
#	endif

	// now add the batched jobs
	for (auto it = vec_paths.begin(), itEnd = vec_paths.end(); it != itEnd; ++it)
		m_jobs.Order(new GalaxyObjectCache<T,CompareT>::CacheJob(std::move(*it), this, callback));
}


template <typename T, typename CompareT>
GalaxyObjectCache<T,CompareT>::CacheJob::CacheJob(std::unique_ptr<std::vector<SystemPath> > path,
	typename GalaxyObjectCache<T,CompareT>::Slave* slaveCache, typename GalaxyObjectCache<T,CompareT>::CacheFilledCallback callback)
	: Job(), m_paths(std::move(path)), m_slaveCache(slaveCache), m_callback(callback)
{
	m_objects.reserve(m_paths->size());
}

//virtual
template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::CacheJob::OnRun()    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
{
	for (auto it = m_paths->begin(), itEnd = m_paths->end(); it != itEnd; ++it)
		m_objects.push_back(RefCountedPtr<T>(new T(*it, nullptr)));
}

//virtual
template <typename T, typename CompareT>
void GalaxyObjectCache<T,CompareT>::CacheJob::OnFinish()  // runs in primary thread of the context
{
	m_slaveCache->AddToCache(m_objects);
	if (m_slaveCache->m_jobs.IsEmpty() && m_callback)
		m_callback();
}

/****** SectorCache ******/

template <> const std::string GalaxyObjectCache<Sector,SystemPath::LessSectorOnly>::CACHE_NAME("SectorCache");

template class GalaxyObjectCache<Sector,SystemPath::LessSectorOnly>;

/****** StarSystemCache ******/

template <>
GalaxyObjectCache<StarSystem,SystemPath::LessSystemOnly>::Slave::Slave(GalaxyObjectCache<StarSystem,SystemPath::LessSystemOnly>* master, JobQueue* jobQueue)
	: m_master(master), m_jobs(Pi::GetSyncJobQueue())
{
	m_master->m_slaves.insert(this);
}

template <> const std::string GalaxyObjectCache<StarSystem,SystemPath::LessSystemOnly>::CACHE_NAME("StarSystemCache");

template class GalaxyObjectCache<StarSystem,SystemPath::LessSystemOnly>;
