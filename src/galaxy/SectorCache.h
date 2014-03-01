// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SECTORCACHE_H
#define SECTORCACHE_H

#include <memory>
#include <map>
#include <set>
#include <vector>
#include "libs.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "JobQueue.h"
#include "RefCounted.h"

class Sector;

class SectorCache {
	friend class Sector;

public:
	RefCountedPtr<Sector> GetCached(const SystemPath& loc);
	RefCountedPtr<Sector> GetIfCached(const SystemPath& loc);
	void ClearCache(); 	// Completely clear slave caches
	void AssignFactions(); // Assign factions to the cached sectors that do not have one, yet
	bool IsEmpty() { return m_sectorAttic.empty(); }

	typedef std::vector<SystemPath> PathVector;
	typedef std::map<SystemPath,RefCountedPtr<Sector> > SectorCacheMap;
	typedef std::map<SystemPath,Sector*> SectorAtticMap;

	class Slave : public RefCounted {
		friend class SectorCache;
	public:
		RefCountedPtr<Sector> GetCached(const SystemPath& loc);
		RefCountedPtr<Sector> GetIfCached(const SystemPath& loc);
		SectorCacheMap::const_iterator Begin() const { return m_sectorCache.begin(); }
		SectorCacheMap::const_iterator End() const { return m_sectorCache.end(); }
		//void Insert(RefCountedPtr<Sector> sec);
		void FillCache(const PathVector& paths);
		void Erase(const SystemPath& loc);
		void Erase(const SectorCacheMap::const_iterator& it);
		void ClearCache();
		bool IsEmpty() { return m_sectorCache.empty(); }
		~Slave();

	private:
		SectorCacheMap m_sectorCache;
		JobSet m_jobs;

		Slave();
		void AddToCache(const std::vector<RefCountedPtr<Sector> >& secIn);
	};

	RefCountedPtr<Slave> NewSlaveCache();

private:
	static const unsigned CACHE_JOB_SIZE = 100;

	void AddToCache(std::vector<RefCountedPtr<Sector> >& sec);
	bool HasCached(const SystemPath& loc) const;
	void RemoveFromAttic(const SystemPath& path);

	// ********************************************************************************
	// Overloaded Job class to handle generating a collection of sectors
	// ********************************************************************************
	class SectorCacheJob : public Job
	{
	public:
		SectorCacheJob(std::unique_ptr<std::vector<SystemPath> > path, Slave* slaveCache);

		virtual void OnRun();    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
		virtual void OnFinish();  // runs in primary thread of the context
		virtual void OnCancel() {}  // runs in primary thread of the context

	private:
		std::unique_ptr<std::vector<SystemPath> > m_paths;
		std::vector<RefCountedPtr<Sector> > m_sectors;
		Slave* m_slaveCache;
	};

	std::set<Slave*> m_slaves;
	SectorAtticMap m_sectorAttic;	// Those contains non-refcounted pointers which are kept alive by RefCountedPtrs in slave caches
									// or elsewhere. The Sector destructor ensures that it is removed from here.
									// This ensures, that there is only ever one object for each Sector.
	std::set<Sector*> m_unassignedFactionsSet;
};

#endif
