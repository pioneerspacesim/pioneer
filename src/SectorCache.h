// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SECTORCACHE_H
#define _SECTORCACHE_H

#include "libs.h"
#include "gui/Gui.h"
#include "View.h"
#include <vector>
#include <set>
#include <string>
#include "View.h"
#include "galaxy/Sector.h"
#include "galaxy/SystemPath.h"
#include "graphics/Drawables.h"

class SectorCache {
public:
	static void Init();
	static void DeInit();

	static Sector* GetCached(const SystemPath& loc);
	static Sector* GetCached(const int sectorX, const int sectorY, const int sectorZ);
	static void ShrinkCache();

	static void SetZoomClamp(const float zoomClamp) { m_zoomClamped = zoomClamp; }
	static void SetPosition(const vector3f& pos) { m_pos = pos; }

	typedef std::map<SystemPath,Sector*> SectorCacheMap;
	typedef SectorCacheMap::const_iterator SectorCacheMapConstIterator;

	static SectorCacheMap& GetCache() { return m_sectorCache; }

private:
	static SectorCacheMap m_sectorCache;

	static int m_cacheXMin;
	static int m_cacheXMax;
	static int m_cacheYMin;
	static int m_cacheYMax;
	static int m_cacheZMin;
	static int m_cacheZMax;
	static float m_zoomClamped;
	static vector3f m_pos;
};

#endif /* _SECTORCACHE_H */
