// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "galaxy/GalaxyCache.h"

#include "Game.h"
#include "Pi.h"
#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"
#include "core/Log.h"
#include "profiler/Profiler.h"
#include <utility>

//#define DEBUG_CACHE

//virtual

template <typename T, typename CompareT>
GalaxyObjectCache<T, CompareT>::~GalaxyObjectCache()
{
	for (Slave *s : m_slaves)
		s->MasterDeleted();
	assert(m_attic.empty()); // otherwise the objects will deregister at a cache that no longer exists
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::AddToCache(std::vector<RefCountedPtr<T>> &objects)
{
	PROFILE_SCOPED()
	for (auto it = objects.begin(), itEnd = objects.end(); it != itEnd; ++it) {
		auto inserted = m_attic.insert(std::make_pair(it->Get()->GetPath(), it->Get()));
		if (!inserted.second) {
			it->Reset(inserted.first->second);
		} else {
			(*it)->SetCache(this);
		}
	}
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T, CompareT>::GetIfCached(const SystemPath &path)
{
	RefCountedPtr<T> s;
	typename AtticMap::iterator i = m_attic.find(path);
	if (i != m_attic.end()) {
		s.Reset(i->second);
	}

	return s;
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T, CompareT>::GetCached(const SystemPath &path)
{
	RefCountedPtr<T> s = this->GetIfCached(path);
	if (!s) {
		++m_cacheMisses;
		s = m_galaxy->GetGenerator()->Generate<T, GalaxyObjectCache<T, CompareT>>(RefCountedPtr<Galaxy>(m_galaxy), path, this);
		m_attic.insert(std::make_pair(path, s.Get()));
	} else {
		++m_cacheHits;
	}
	return s;
}

template <typename T, typename CompareT>
bool GalaxyObjectCache<T, CompareT>::HasCached(const SystemPath &path) const
{
	PROFILE_SCOPED()

	return (m_attic.find(path) != m_attic.end());
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::RemoveFromAttic(const SystemPath &path)
{
	m_attic.erase(path);
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::ClearCache()
{
	for (auto it = m_slaves.begin(), itEnd = m_slaves.end(); it != itEnd; ++it)
		(*it)->ClearCache();
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::OutputCacheStatistics(bool reset)
{
	Output("%s: misses: %llu, slave hits: %llu, master hits: %llu\n", CACHE_NAME.c_str(), m_cacheMisses, m_cacheHitsSlave, m_cacheHits);
	if (reset)
		m_cacheMisses = m_cacheHitsSlave = m_cacheHits = 0;
}

template <typename T, typename CompareT>
RefCountedPtr<typename GalaxyObjectCache<T, CompareT>::Slave> GalaxyObjectCache<T, CompareT>::NewSlaveCache()
{
	return RefCountedPtr<Slave>(new Slave(this, RefCountedPtr<Galaxy>(m_galaxy), Pi::GetAsyncJobQueue()));
}

template <typename T, typename CompareT>
GalaxyObjectCache<T, CompareT>::Slave::Slave(GalaxyObjectCache<T, CompareT> *master, RefCountedPtr<Galaxy> galaxy, JobQueue *jobQueue) :
	m_master(master),
	m_galaxy(galaxy),
	m_jobs(Pi::GetAsyncJobQueue())
{
	m_master->m_slaves.insert(this);
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::Slave::MasterDeleted()
{
	m_master = nullptr;
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T, CompareT>::Slave::GetIfCached(const SystemPath &path)
{
	typename CacheMap::iterator i = m_cache.find(path);
	if (i != m_cache.end())
		return (*i).second;
	return RefCountedPtr<T>();
}

template <typename T, typename CompareT>
RefCountedPtr<T> GalaxyObjectCache<T, CompareT>::Slave::GetCached(const SystemPath &path)
{
	PROFILE_SCOPED()

	typename CacheMap::iterator i = m_cache.find(path);
	if (i != m_cache.end()) {
		if (m_master)
			++m_master->m_cacheHitsSlave;
		return (*i).second;
	}

	if (m_master) {
		auto inserted = m_cache.insert(std::make_pair(path, m_master->GetCached(path)));
		return inserted.first->second;
	} else {
		return RefCountedPtr<T>();
	}
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::Slave::Erase(const SystemPath &path) { m_cache.erase(path); }

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::Slave::Erase(const typename CacheMap::const_iterator &it) { m_cache.erase(it); }

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::Slave::ClearCache() { m_cache.clear(); }

template <typename T, typename CompareT>
GalaxyObjectCache<T, CompareT>::Slave::~Slave()
{
#ifdef DEBUG_CACHE
	unsigned unique = 0;
	for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
		if (it->second->GetRefCount() == 1)
			unique++;
	Output("%s: Discarding slave cache with " SIZET_FMT " entries (%u to be removed)\n", CACHE_NAME.c_str(), m_cache.size(), unique);
#endif
	if (m_master)
		m_master->m_slaves.erase(this);
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::Slave::AddToCache(std::vector<RefCountedPtr<T>> &objects)
{
	if (m_master) {
		m_master->AddToCache(objects); // This modifies the vector to the sectors already in the master cache
		for (auto it = objects.begin(), itEnd = objects.end(); it != itEnd; ++it) {
			m_cache.insert(std::make_pair(it->Get()->GetPath(), *it));
		}
	}
}

template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::Slave::FillCache(const typename GalaxyObjectCache<T, CompareT>::PathVector &paths,
	typename GalaxyObjectCache<T, CompareT>::CacheFilledCallback callback)
{
	// allocate some space for what we're about to chunk up
	std::vector<std::unique_ptr<PathVector>> vec_paths;
	vec_paths.reserve(paths.size() / CACHE_JOB_SIZE + 1);
	std::unique_ptr<PathVector> current_paths;
#ifdef DEBUG_CACHE
	size_t alreadyCached = m_cache.size();
	unsigned masterCached = 0;
	unsigned toBeCreated = 0;
#endif

	// chop the paths into groups of CACHE_JOB_SIZE
	for (auto it = paths.begin(), itEnd = paths.end(); it != itEnd; ++it) {
		RefCountedPtr<T> s = m_master->GetIfCached(*it);
		if (s) {
			m_cache[*it] = s;
#ifdef DEBUG_CACHE
			++masterCached;
#endif
		} else {
			if (!current_paths) {
				current_paths.reset(new PathVector);
				current_paths->reserve(CACHE_JOB_SIZE);
			}
			current_paths->push_back(*it);
			if (current_paths->size() >= CACHE_JOB_SIZE) {
				vec_paths.push_back(std::move(current_paths));
			}
#ifdef DEBUG_CACHE
			++toBeCreated;
#endif
		}
	}

	// catch the last loop in case it's got some entries (could be less than the spread width)
	if (current_paths) {
		vec_paths.push_back(std::move(current_paths));
	}

#ifdef DEBUG_CACHE
	Output("%s: FillCache: " SIZET_FMT " cached, %u in master cache, %u to be created, will use " SIZET_FMT " jobs\n", CACHE_NAME.c_str(),
		alreadyCached, masterCached, toBeCreated, vec_paths.size());
#endif

	if (vec_paths.empty()) {
		if (callback)
			callback();
	} else {
		// now add the batched jobs
		for (auto it = vec_paths.begin(), itEnd = vec_paths.end(); it != itEnd; ++it)
			m_jobs.Order(new GalaxyObjectCache<T, CompareT>::CacheJob(std::move(*it), this, m_galaxy, callback));
	}
}

template <typename T, typename CompareT>
GalaxyObjectCache<T, CompareT>::CacheJob::CacheJob(std::unique_ptr<std::vector<SystemPath>> path,
	typename GalaxyObjectCache<T, CompareT>::Slave *slaveCache, RefCountedPtr<Galaxy> galaxy,
	typename GalaxyObjectCache<T, CompareT>::CacheFilledCallback callback) :
	Job(),
	m_paths(std::move(path)),
	m_slaveCache(slaveCache),
	m_galaxy(galaxy),
	m_galaxyGenerator(galaxy->GetGenerator()),
	m_callback(callback)
{
	m_objects.reserve(m_paths->size());
}

//virtual
template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::CacheJob::OnRun() // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
{
	PROFILE_SCOPED()
	for (auto it = m_paths->begin(), itEnd = m_paths->end(); it != itEnd; ++it)
		m_objects.push_back(m_galaxyGenerator->Generate<T, GalaxyObjectCache<T, CompareT>>(m_galaxy, *it, nullptr));
}

//virtual
template <typename T, typename CompareT>
void GalaxyObjectCache<T, CompareT>::CacheJob::OnFinish() // runs in primary thread of the context
{
	PROFILE_SCOPED()
	m_slaveCache->AddToCache(m_objects);
	if (m_slaveCache->m_jobs.IsEmpty() && m_callback)
		m_callback();
}

/****** SectorCache ******/

template <>
const std::string GalaxyObjectCache<Sector, SystemPath::LessSectorOnly>::CACHE_NAME("SectorCache");

template class GalaxyObjectCache<Sector, SystemPath::LessSectorOnly>;

/****** StarSystemCache ******/

template <>
GalaxyObjectCache<StarSystem, SystemPath::LessSystemOnly>::Slave::Slave(GalaxyObjectCache<StarSystem, SystemPath::LessSystemOnly> *master, RefCountedPtr<Galaxy> galaxy, JobQueue *jobQueue) :
	m_master(master),
	m_galaxy(galaxy),
	m_jobs(Pi::GetSyncJobQueue())
{
	m_master->m_slaves.insert(this);
}

template <>
const std::string GalaxyObjectCache<StarSystem, SystemPath::LessSystemOnly>::CACHE_NAME("StarSystemCache");

template class GalaxyObjectCache<StarSystem, SystemPath::LessSystemOnly>;
