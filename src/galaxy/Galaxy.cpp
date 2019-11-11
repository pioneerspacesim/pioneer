// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Galaxy.h"

#include "FileSystem.h"
#include "GalaxyGenerator.h"
#include "GameSaveError.h"
#include "Json.h"
#include "Sector.h"
#include "utils.h"

Galaxy::Galaxy(RefCountedPtr<GalaxyGenerator> galaxyGenerator, float radius, float sol_offset_x, float sol_offset_y,
	const std::string &factionsDir, const std::string &customSysDir) :
	GALAXY_RADIUS(radius),
	SOL_OFFSET_X(sol_offset_x),
	SOL_OFFSET_Y(sol_offset_y),
	m_initialized(false),
	m_galaxyGenerator(galaxyGenerator),
	m_sectorCache(this),
	m_starSystemCache(this),
	m_factions(this, factionsDir),
	m_customSystems(this, customSysDir)
{
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
	m_customSystems.Init();
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

// sort using a custom function object
class SectorDistanceSort {
public:
	SectorDistanceSort() = delete;

	bool operator()(const SystemPath &a, const SystemPath &b)
	{
		const float dist_a = vector3f(m_here.sectorX - a.sectorX, m_here.sectorY - a.sectorY, m_here.sectorZ - a.sectorZ).LengthSqr();
		const float dist_b = vector3f(m_here.sectorX - b.sectorX, m_here.sectorY - b.sectorY, m_here.sectorZ - b.sectorZ).LengthSqr();
		return dist_a < dist_b;
	}
	SectorDistanceSort(const SystemPath &centre) :
		m_here(centre)
	{}

private:
	SystemPath m_here;
};

size_t Galaxy::FillSectorCache(RefCountedPtr<SectorCache::Slave> &sc, const SystemPath &center,
	int sectorRadius, SectorCache::CacheFilledCallback callback)
{
	const int here_x = center.sectorX;
	const int here_y = center.sectorY;
	const int here_z = center.sectorZ;

	SectorCache::PathVector paths;

	// build all of the possible paths we'll need to build sectors for
	paths.reserve((sectorRadius * 2 + 1) * (sectorRadius * 2 + 1) * (sectorRadius * 2 + 1));
	for (int x = here_x - sectorRadius; x <= here_x + sectorRadius; x++) {
		for (int y = here_y - sectorRadius; y <= here_y + sectorRadius; y++) {
			for (int z = here_z - sectorRadius; z <= here_z + sectorRadius; z++) {
				paths.emplace_back(x, y, z);
			}
		}
	}
	// sort them so that those closest to the "here" path are processed first
	SectorDistanceSort SDS(center);
	std::sort(paths.begin(), paths.end(), SDS);
	sc->FillCache(paths, callback);
	return paths.size();
}

size_t Galaxy::FillStarSystemCache(RefCountedPtr<StarSystemCache::Slave> &ssc, const SystemPath &center,
		int sectorRadius, RefCountedPtr<SectorCache::Slave> &source)
{
	const int here_x = center.sectorX;
	const int here_y = center.sectorY;
	const int here_z = center.sectorZ;

	SectorCache::PathVector paths;

	// build all of the possible paths we'll need to build StarSystem for
	paths.reserve((sectorRadius * 2 + 1) * (sectorRadius * 2 + 1) * (sectorRadius * 2 + 1));
	for (int x = here_x - sectorRadius; x <= here_x + sectorRadius; x++) {
		for (int y = here_y - sectorRadius; y <= here_y + sectorRadius; y++) {
			for (int z = here_z - sectorRadius; z <= here_z + sectorRadius; z++) {
				SystemPath path(x, y, z);
				RefCountedPtr<Sector> sec(source->GetIfCached(path));
				assert(!sec.Valid());
				for (const Sector::System &ss : sec->m_systems)
					paths.emplace_back(ss.sx, ss.sy, ss.sz, ss.idx);
			}
		}
	}
	ssc->FillCache(paths);
	return paths.size();
}

RefCountedPtr<StarSystem> Galaxy::GetStarSystem(const SystemPath &path)
{
	return m_starSystemCache.GetCached(path);
}

vector3d Galaxy::GetInterSystemPosition(const SystemPath &source, const SystemPath &dest)
{
	RefCountedPtr<const Sector> source_sec = GetSector(source);
	RefCountedPtr<const Sector> dest_sec = GetSector(dest);

	Sector::System source_sys = source_sec->m_systems[source.systemIndex];
	Sector::System dest_sys = dest_sec->m_systems[dest.systemIndex];

	const vector3d sourcePos(source_sys.GetFullPosition());
	const vector3d destPos(dest_sys.GetFullPosition());
	return destPos - sourcePos;
}

DensityMapGalaxy::DensityMapGalaxy(RefCountedPtr<GalaxyGenerator> galaxyGenerator, const std::string &mapfile,
	float radius, float sol_offset_x, float sol_offset_y, const std::string &factionsDir, const std::string &customSysDir) :
	Galaxy(galaxyGenerator, radius, sol_offset_x, sol_offset_y, factionsDir, customSysDir),
	m_mapWidth(0),
	m_mapHeight(0)
{
	RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(mapfile);
	if (!filedata) {
		Error("Galaxy: couldn't load '%s'\n", mapfile.c_str());
	}

	SDL_RWops *datastream = SDL_RWFromConstMem(filedata->GetData(), filedata->GetSize());
	SDL_Surface *galaxyImg = SDL_LoadBMP_RW(datastream, 1);
	if (!galaxyImg) {
		Error("Galaxy: couldn't load: %s (%s)\n", mapfile.c_str(), SDL_GetError());
	}

	// now that we have our raw image loaded
	// allocate the space for our processed representation
	m_galaxyMap.reset(new float[(galaxyImg->w * galaxyImg->h)]);
	// lock the image once so we can read from it
	SDL_LockSurface(galaxyImg);
	// setup our map dimensions for later
	m_mapWidth = galaxyImg->w;
	m_mapHeight = galaxyImg->h;
	// copy every pixel value from the red channel (image is greyscale, channel is irrelevant)
	for (int x = 0; x < galaxyImg->w; x++) {
		for (int y = 0; y < galaxyImg->h; y++) {
			const float val = float(static_cast<unsigned char *>(galaxyImg->pixels)[x + y * galaxyImg->pitch]);
			m_galaxyMap.get()[x + y * m_mapWidth] = val;
		}
	}
	// unlock the surface and then release it
	SDL_UnlockSurface(galaxyImg);
	if (galaxyImg)
		SDL_FreeSurface(galaxyImg);
}

static const float one_over_256(1.0f / 256.0f);
Uint8 DensityMapGalaxy::GetSectorDensity(const int sx, const int sy, const int sz) const
{
	// -1.0 to 1.0 then limited to 0.0 to 1.0
	const float offset_x = (((sx * Sector::SIZE + SOL_OFFSET_X) / GALAXY_RADIUS) + 1.0f) * 0.5f;
	const float offset_y = (((-sy * Sector::SIZE + SOL_OFFSET_Y) / GALAXY_RADIUS) + 1.0f) * 0.5f;

	const int x = int(floor(offset_x * (m_mapWidth - 1)));
	const int y = int(floor(offset_y * (m_mapHeight - 1)));

	float val = m_galaxyMap.get()[x + y * m_mapWidth];

	// crappy unrealistic but currently adequate density dropoff with sector z
	val = val * (256.0f - std::min(float(abs(sz)), 256.0f)) * one_over_256;

	// reduce density somewhat to match real (gliese) density
	val *= 0.5f;

	return Uint8(val);
}
