// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SECTORCACHE_H
#define SECTORCACHE_H

#include <memory>
#include "libs.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "JobQueue.h"
#include "RefCounted.h"

class Sector;

class SectorCache {
	friend class Sector;

public:
	SectorCache();

	RefCountedPtr<Sector> GetCached(const SystemPath& loc);
	void GenSectorCache();
	void ShrinkCache();	// shrink cache to the surrounding of the player (putting still referenced Sectors to attic).
	void ClearCache(); 	// Completely clear cache, putting everything to attic.
	bool IsCompletelyEmpty() { return m_sectorCache.empty() && m_sectorAttic.empty(); }

	void SetZoomClamp(const float zoomClamp) { m_zoomClamped = zoomClamp; }
	void SetPosition(const vector3f& pos) { m_pos = pos; }

	typedef std::map<SystemPath,RefCountedPtr<Sector> > SectorCacheMap;
	typedef std::map<SystemPath,Sector*> SectorAtticMap;
	typedef SectorCacheMap::const_iterator SectorCacheMapConstIterator;

	SectorCacheMapConstIterator Begin() { return m_sectorCache.begin(); }
	SectorCacheMapConstIterator End() { return m_sectorCache.end(); }

private:
	void AddToCache(const std::vector<RefCountedPtr<Sector> >& secIn);
	bool HasCached(const SystemPath& loc) const;
	bool HasCached(const SystemPath& loc, bool revive);
	void RemoveFromAttic(const SystemPath& path);

	// ********************************************************************************
	// Overloaded Job class to handle generating a collection of sectors
	// ********************************************************************************
	class SectorCacheJob : public Job
	{
	public:
		SectorCacheJob(std::unique_ptr<std::vector<SystemPath> > path);
		virtual ~SectorCacheJob() {}

		virtual void OnRun();    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
		virtual void OnFinish();  // runs in primary thread of the context
		virtual void OnCancel() {}   // runs in primary thread of the context

	protected:
		std::unique_ptr<std::vector<SystemPath> > m_paths;
		std::vector<RefCountedPtr<Sector> > m_sectors;
	};

	SectorCacheMap m_sectorCache;
	SectorAtticMap m_sectorAttic; // Those contains non-refcounted pointers to the Sectors the cache
								  // is no longer interested in, but still exist due to another attachement.
								  // This ensures, that there is only ever one object for each Sector.
								  // The Sector destructor ensures that it is removed from attic.

	int m_cacheXMin;
	int m_cacheXMax;
	int m_cacheYMin;
	int m_cacheYMax;
	int m_cacheZMin;
	int m_cacheZMax;
	float m_zoomClamped;
	vector3f m_pos;
};

#endif
