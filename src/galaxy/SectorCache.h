// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SECTORCACHE_H
#define SECTORCACHE_H

#include "libs.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"
#include "JobQueue.h"

class Sector;

class SectorCache {
public:
	SectorCache();
	~SectorCache();

	void AddToCache(const std::vector<Sector>& secIn);
	Sector* GetCached(const SystemPath& loc);
	bool HasCached(const SystemPath& loc) const;
	void ShrinkCache();

	void SetZoomClamp(const float zoomClamp) { m_zoomClamped = zoomClamp; }
	void SetPosition(const vector3f& pos) { m_pos = pos; }

	typedef std::map<SystemPath,Sector*> SectorCacheMap;
	typedef SectorCacheMap::const_iterator SectorCacheMapConstIterator;

	SectorCacheMapConstIterator Begin() { return m_sectorCache.begin(); }
	SectorCacheMapConstIterator End() { return m_sectorCache.end(); }

private:
	SectorCacheMap m_sectorCache;

	int m_cacheXMin;
	int m_cacheXMax;
	int m_cacheYMin;
	int m_cacheYMax;
	int m_cacheZMin;
	int m_cacheZMax;
	float m_zoomClamped;
	vector3f m_pos;
};

// ********************************************************************************
// Overloaded Job class to handle generating a collection of sectors
// ********************************************************************************
class SectorCacheJob : public Job
{
public:
	SectorCacheJob(const std::vector<SystemPath>& path);
	virtual ~SectorCacheJob() {}

	virtual void OnRun();    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	virtual void OnFinish();  // runs in primary thread of the context
	virtual void OnCancel() {}   // runs in primary thread of the context

protected:
	std::vector<SystemPath> m_paths;
	std::vector<Sector> m_sectors;
};

#endif
