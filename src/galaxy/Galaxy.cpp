// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "utils.h"
#include "Galaxy.h"
#include "Sector.h"
#include "Pi.h"
#include "FileSystem.h"

Galaxy::Galaxy() : GALAXY_RADIUS(50000.0), SOL_OFFSET_X(25000.0), SOL_OFFSET_Y(0.0), m_galaxybmp(nullptr), m_factions(this), m_customSystems(this)
{
	static const std::string filename("galaxy.bmp");

	RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(filename);
	if (!filedata) {
		Output("Galaxy: couldn't load '%s'\n", filename.c_str());
		Pi::Quit();
	}

	SDL_RWops *datastream = SDL_RWFromConstMem(filedata->GetData(), filedata->GetSize());
	m_galaxybmp = SDL_LoadBMP_RW(datastream, 1);
	if (!m_galaxybmp) {
		Output("Galaxy: couldn't load: %s (%s)\n", filename.c_str(), SDL_GetError());
		Pi::Quit();
	}

	m_starSystemCache = m_starSystemAttic.NewSlaveCache();
}

Galaxy::~Galaxy()
{
	if(m_galaxybmp) SDL_FreeSurface(m_galaxybmp);
}

void Galaxy::Init()
{
	m_customSystems.Init();
	m_factions.Init();
}

SDL_Surface* Galaxy::GetGalaxyBitmap()
{
	return m_galaxybmp;
}

Uint8 Galaxy::GetSectorDensity(int sx, int sy, int sz)
{
	// -1.0 to 1.0
	float offset_x = (sx*Sector::SIZE + SOL_OFFSET_X)/GALAXY_RADIUS;
	float offset_y = (-sy*Sector::SIZE + SOL_OFFSET_Y)/GALAXY_RADIUS;
	// 0.0 to 1.0
	offset_x = Clamp((offset_x + 1.0)*0.5, 0.0, 1.0);
	offset_y = Clamp((offset_y + 1.0)*0.5, 0.0, 1.0);

	int x = int(floor(offset_x * (m_galaxybmp->w - 1)));
	int y = int(floor(offset_y * (m_galaxybmp->h - 1)));

	SDL_LockSurface(m_galaxybmp);
	int val = static_cast<unsigned char*>(m_galaxybmp->pixels)[x + y*m_galaxybmp->pitch];
	SDL_UnlockSurface(m_galaxybmp);
	// crappy unrealistic but currently adequate density dropoff with sector z
	val = val * (256 - std::min(abs(sz),256)) / 256;
	// reduce density somewhat to match real (gliese) density
	val /= 2;
	return Uint8(val);
}

void Galaxy::FlushCaches()
{
	m_starSystemAttic.OutputCacheStatistics();
	m_starSystemCache = m_starSystemAttic.NewSlaveCache();
	m_starSystemAttic.ClearCache();
	m_sectorCache.OutputCacheStatistics();
	m_sectorCache.ClearCache();
	// XXX Ideally the cache would now be empty, but we still have Faction::m_homesector :(
	// assert(m_sectorCache.IsEmpty());
}

void Galaxy::Dump(FILE* file, Sint32 centerX, Sint32 centerY, Sint32 centerZ, Sint32 radius)
{
	for (Sint32 sx = centerX - radius; sx <= centerX + radius; ++sx) {
		for (Sint32 sy = centerY - radius; sy <= centerY + radius; ++sy) {
			for (Sint32 sz = centerZ - radius; sz <= centerZ + radius; ++sz) {
				RefCountedPtr<const Sector> sector = Pi::GetGalaxy()->GetSector(SystemPath(sx, sy, sz));
				sector->Dump(file);
			}
			m_starSystemAttic.ClearCache();
		}
	}
}
