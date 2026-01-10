// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Galaxy.h"

#include "FileSystem.h"
#include "GalaxyGenerator.h"
#include "GameSaveError.h"
#include "Json.h"
#include "MathUtil.h"
#include "Sector.h"
#include "core/Log.h"

#include <SDL_surface.h>

Galaxy::Galaxy(RefCountedPtr<GalaxyGenerator> galaxyGenerator, float radius, float sol_offset_x, float sol_offset_y,
	const std::string &factionsDir, const std::string &customSysDir) :
	GALAXY_RADIUS(radius),
	SOL_OFFSET_X(sol_offset_x),
	SOL_OFFSET_Y(sol_offset_y),
	m_initialized(false),
	m_stats(),
	m_galaxyGenerator(galaxyGenerator),
	m_sectorCache(this),
	m_starSystemCache(this),
	m_factions(this, factionsDir),
	m_customSystems(this, customSysDir)
{
	m_stats.EnableReset(false);
}

//static
RefCountedPtr<Galaxy> Galaxy::LoadFromJson(const Json &jsonObj)
{
	if (!jsonObj.count("galaxy_generator")) throw SavedGameCorruptException();
	Json galaxyGenObj = jsonObj["galaxy_generator"];

	RefCountedPtr<Galaxy> galaxy = GalaxyGenerator::CreateFromJson(galaxyGenObj);
	galaxy->m_galaxyGenerator->FromJson(galaxyGenObj, galaxy);
	return galaxy;
}

void Galaxy::ToJson(Json &jsonObj)
{
	m_galaxyGenerator->ToJson(jsonObj, RefCountedPtr<Galaxy>(this));
}

void Galaxy::SetGalaxyGenerator(RefCountedPtr<GalaxyGenerator> galaxyGenerator)
{
	m_galaxyGenerator = galaxyGenerator;
}

Galaxy::~Galaxy()
{
}

void Galaxy::Init()
{
	m_customSystems.Load();
	m_factions.Init();
	m_initialized = true;
	m_factions.PostInit(); // So, cached home sectors take persisted state into account
#if 0
	{
		Profiler::Timer timer;
		timer.Start();
		Uint32 totalVal = 0;
		const static int s_count = 64;
		for( int sx=-s_count; sx<s_count; sx++ ) {
			for( int sy=-s_count; sy<s_count; sy++ ) {
				for( int sz=-s_count; sz<s_count; sz++ ) {
					totalVal += GetSectorDensity( sx, sy, sz );
				}
			}
		}
		timer.Stop();
		Output("\nGalaxy test took: %lf milliseconds, totalVal (%u)\n", timer.millicycles(), totalVal);
	}
#endif
}

void Galaxy::FlushCaches()
{
	m_factions.ClearCache();
	m_starSystemCache.OutputCacheStatistics();
	m_starSystemCache.ClearCache();
	m_sectorCache.OutputCacheStatistics();
	m_sectorCache.ClearCache();
	assert(m_sectorCache.IsEmpty());
}

void Galaxy::Dump(FILE *file, Sint32 centerX, Sint32 centerY, Sint32 centerZ, Sint32 radius)
{
	for (Sint32 sx = centerX - radius; sx <= centerX + radius; ++sx) {
		for (Sint32 sy = centerY - radius; sy <= centerY + radius; ++sy) {
			for (Sint32 sz = centerZ - radius; sz <= centerZ + radius; ++sz) {
				RefCountedPtr<const Sector> sector = GetSector(SystemPath(sx, sy, sz));
				sector->Dump(file);
			}
			m_starSystemCache.ClearCache();
		}
	}
}

RefCountedPtr<GalaxyGenerator> Galaxy::GetGenerator() const
{
	return m_galaxyGenerator;
}

const std::string &Galaxy::GetGeneratorName() const
{
	return m_galaxyGenerator->GetName();
}

int Galaxy::GetGeneratorVersion() const
{
	return m_galaxyGenerator->GetVersion();
}

DensityMapGalaxy::DensityMapGalaxy(RefCountedPtr<GalaxyGenerator> galaxyGenerator, const std::string &mapfile,
	float radius, float sol_offset_x, float sol_offset_y, const std::string &factionsDir, const std::string &customSysDir) :
	Galaxy(galaxyGenerator, radius, sol_offset_x, sol_offset_y, factionsDir, customSysDir),
	m_mapWidth(0),
	m_mapHeight(0)
{
	// None of our subsystems are fully initialized at galaxy load time and we
	// need access to the raw pixel data rather than a GPU texture.
	// Could still benefit from being loaded through a resource system instead.
	RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(mapfile);
	if (!filedata) {
		Error("Galaxy: couldn't load '%s'\n", mapfile.c_str());
	}

	SDL_RWops *datastream = SDL_RWFromConstMem(filedata->GetData(), filedata->GetSize());
	SDL_Surface *galaxyImg = SDL_LoadBMP_RW(datastream, 1);
	if (!galaxyImg) {
		Error("Galaxy: couldn't load: %s (%s)\n", mapfile.c_str(), SDL_GetError());
	}

	if (galaxyImg->format->BytesPerPixel != 1) {
		Error("Galaxy: density map image is not a grayscale bitmap!");
	}

	// now that we have our raw image loaded
	// allocate the space for our processed representation
	m_galaxyMap.reset(new uint8_t[(galaxyImg->w * galaxyImg->h)]);
	// lock the image once so we can read from it
	SDL_LockSurface(galaxyImg);
	// setup our map dimensions for later
	m_mapWidth = galaxyImg->w;
	m_mapHeight = galaxyImg->h;

	// copy every pixel value from the image - the image must be in a 1Bpp grayscale image format
	for (int y = 0; y < galaxyImg->h; y++) {
		for (int x = 0; x < galaxyImg->w; x++) {
			const uint8_t val = static_cast<uint8_t *>(galaxyImg->pixels)[x + y * galaxyImg->pitch];
			m_galaxyMap.get()[x + y * m_mapWidth] = val;
		}
	}

	// unlock the surface and then release it
	SDL_UnlockSurface(galaxyImg);
	if (galaxyImg)
		SDL_FreeSurface(galaxyImg);
}

static const float one_over_256(1.0f / 256.0f);

// Simple clamped texture sample
static float sample(uint8_t pixels[], float x, float y, uint32_t w, uint32_t h)
{
	int tex_x = Clamp<int>(floor(x), 0, w);
	int tex_y = Clamp<int>(floor(y), 0, h);

	return pixels[tex_x + tex_y * w] * one_over_256;
}

Uint8 DensityMapGalaxy::GetSectorDensity(const int sx, const int sy, const int sz) const
{
	// -1.0 to 1.0 then limited to 0.0 to 1.0
	const float offset_x = (((sx * Sector::SIZE + SOL_OFFSET_X) / GALAXY_RADIUS) + 1.0f) * 0.5f;
	const float offset_y = (((-sy * Sector::SIZE + SOL_OFFSET_Y) / GALAXY_RADIUS) + 1.0f) * 0.5f;

	const float fx = offset_x * (m_mapWidth - 1);
	const float fy = offset_y * (m_mapHeight - 1);
	const float mf = std::fmodf(fx, 1.f);
	const float my = std::fmodf(fy, 1.f);

	// Sample four times against the density map texture (clamped internally)
	const float val1 = sample(m_galaxyMap.get(), fx, fy, m_mapWidth, m_mapHeight);
	const float val2 = sample(m_galaxyMap.get(), fx + 1.0f, fy, m_mapWidth, m_mapHeight);
	const float val3 = sample(m_galaxyMap.get(), fx, fy + 1.0f, m_mapWidth, m_mapHeight);
	const float val4 = sample(m_galaxyMap.get(), fx + 1.0f, fy + 1.0f, m_mapWidth, m_mapHeight);

	// Do a bilinear interpolation
	const float interp_1 = val1 * (1.f - mf) + val2 * mf;
	const float interp_2 = val3 * (1.f - mf) + val4 * mf;

	float val = interp_1 * (1.f - my) + interp_2 * my;

	// Slightly improved and currently adequate density dropoff with sector z
	// Produces ~1000ly thick "arms" and a significant bulge in the center
	const float dropoff_scale = std::max(1.f, 3.5f - val * 2.5f);
	const float dropoff = 1.0f - Clamp(std::min(abs(sz), 256) * one_over_256 * dropoff_scale, 0.f, 1.f);

	val = val * dropoff;

	// Return an integer value normalized to 256... sigh
	return Clamp<uint8_t>(val * 256.0f, 0, 255);
}
