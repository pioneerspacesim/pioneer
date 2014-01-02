// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef SECTORCACHE_H
#define SECTORCACHE_H

#include "libs.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"

class Sector;

class SectorCache {
public:
	SectorCache();
	~SectorCache();

	Sector* GetCached(const SystemPath& loc);
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

#endif
