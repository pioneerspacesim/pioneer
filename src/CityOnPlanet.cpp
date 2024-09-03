// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CityOnPlanet.h"

#include "FileSystem.h"
#include "Frame.h"
#include "Game.h"
#include "JsonUtils.h"
#include "ModelCache.h"
#include "Pi.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "collider/Geom.h"
#include "core/Log.h"

#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Types.h"

#include "scenegraph/Animation.h"
#include "scenegraph/ModelSkin.h"
#include "scenegraph/SceneGraph.h"
#include "utils.h"

#include "utils.h"

std::vector<CityOnPlanet::CityFlavourType> CityOnPlanet::s_cityFlavours;
std::unique_ptr<Graphics::Material> CityOnPlanet::s_debugMat;

void CityOnPlanet::AddStaticGeomsToCollisionSpace()
{
	PROFILE_SCOPED()

	size_t numBuildingTypes = m_cityType->buildingTypes.size();

	// reset data structures
	m_enabledBuildings.clear();
	m_buildingCounts.resize(numBuildingTypes);
	for (Uint32 i = 0; i < numBuildingTypes; i++) {
		m_buildingCounts[i] = 0;
	}

	// Generate the new building list
	int skipMask;
	switch (Pi::detail.cities) {
	case 0: skipMask = 0xf; break;
	case 1: skipMask = 0x7; break;
	case 2: skipMask = 0x3; break;
	case 3: skipMask = 0x1; break;
	default:
		skipMask = 0;
		break;
	}
	Uint32 numVisibleBuildings = 0;
	for (unsigned int i = 0; i < m_buildings.size(); i++) {
		if (!(i & skipMask)) {
			++numVisibleBuildings;
		}
	}

	// we know how many building we'll be adding, reserve space up front
	m_enabledBuildings.reserve(numVisibleBuildings);
	for (unsigned int i = 0; i < m_buildings.size(); i++) {
		if (i & skipMask) {
		} else {
			Frame *f = Frame::GetFrame(m_frame);
			f->AddStaticGeom(m_buildings[i].geom);
			m_enabledBuildings.push_back(m_buildings[i]);
			// Update building types
			++(m_buildingCounts[m_buildings[i].instIndex]);
		}
	}

	// reset the reset flag
	m_detailLevel = Pi::detail.cities;
}

void CityOnPlanet::RemoveStaticGeomsFromCollisionSpace()
{
	m_enabledBuildings.clear();
	for (unsigned int i = 0; i < m_buildings.size(); i++) {
		Frame *f = Frame::GetFrame(m_frame);
		f->RemoveStaticGeom(m_buildings[i].geom);
	}
}

void CityOnPlanet::GetModelSize(const Aabb &aabb, uint8_t size[2])
{
	vector3d aabbSize = aabb.max - aabb.min;

	size[0] = std::ceil(aabbSize.x / double(CELLSIZE));
	size[1] = std::ceil(aabbSize.z / double(CELLSIZE));
}

void CityOnPlanet::LoadBuildingType(std::string_view key, const Json &buildingDef, BuildingType &out)
{
	std::string modelPath = buildingDef.value("model", "");

	if (modelPath.empty()) {
		modelPath = key;
	}

	out.model = Pi::FindModel(modelPath);

	if (!out.model->GetCollisionMesh().Get()) {
		out.model->CreateCollisionMesh();
	}

	GetModelSize(out.model->GetCollisionMesh()->GetAabb(), out.cellSize);

	out.cellSize[0] = buildingDef.value("size-x", out.cellSize[0]);
	out.cellSize[1] = buildingDef.value("size-y", out.cellSize[1]);

	if (out.cellSize[0] > CELLMAX || out.cellSize[1] > CELLMAX) {
		Log::Warning("\tCity building {} has {}x{}c footprint (at {} m/c), will be truncated to {}x{}c.",
			key, out.cellSize[0], out.cellSize[1], CELLSIZE, CELLMAX, CELLMAX);

		out.cellSize[0] = std::min(out.cellSize[0], uint8_t(CELLMAX));
		out.cellSize[1] = std::min(out.cellSize[1], uint8_t(CELLMAX));
	}

	out.rarityAirless = buildingDef.value("airless-rarity", 1.0);
	out.rarityAtmo = buildingDef.value("atmo-rarity", 1.0);

	std::string kind = buildingDef.value("kind", "");

	out.buildingKind = SectorKind::None;
	if (kind == "storage")
		out.buildingKind = SectorKind::Storage;
	else if (kind == "industry")
		out.buildingKind = SectorKind::Industry;
	else if (kind == "monument")
		out.buildingKind = SectorKind::Monument;
	else if (kind == "habitat")
		out.buildingKind = SectorKind::Habitat;
	else if (kind == "frontier")
		out.buildingKind = SectorKind::Frontier;

	std::string idleName = buildingDef.value("idle", "");
	if (!idleName.empty()) {
		out.idleAnimation = out.model->FindAnimation(idleName);

		if (out.idleAnimation == nullptr) {
			Log::Warning("\tIdle animation {} specified for building {} not found!", idleName, key);
		}
	} else {
		out.idleAnimation = out.model->FindAnimation("idle");
	}

	Log::Verbose("\tLoaded city building {} ({}x{}c, atm: {}, arl: {}, kind: {}, idleAnim: {})",
		key, out.cellSize[0], out.cellSize[1], out.rarityAtmo, out.rarityAirless, int(out.buildingKind), out.idleAnimation != nullptr);
}

void CityOnPlanet::LoadCityFlavour(const FileSystem::FileInfo &file)
{
	Json fileData = JsonUtils::LoadJsonDataFile(file.GetPath(), true);
	if (!fileData.is_object()) {
		Log::Info("CityOnPlanet: Could not load city definition file '{}' as a valid JSON file.", file.GetPath());
		return;
	}

	Log::Info("Loading city definition file '{}'", file.GetPath());

	const Json &sizeDef = fileData["size"];
	if (!sizeDef.is_array()) {
		Log::Warning("\tMissing city size definition array! Should be \"size\": [ ... ]");
		return;
	}

	const Json &buildingDefs = fileData["buildings"];
	if (!buildingDefs.is_object()) {
		Log::Warning("\tMissing city buildings object! Should be \"buildings\": { ... }");
		return;
	}

	CityFlavourType cityType = {};

	uint32_t numSizeDefs = 0;
	for (const Json &item : sizeDef) {
		CityRadiusDef radius = {};
		numSizeDefs++;

		radius.population = item.value("population", 0.0);
		radius.baseSize = item.value("baseSize", 0.0);
		radius.atmoSize = item.value("atmoSize", 0.0);
		radius.randomSize = item.value("randomSize", 0.0);
		radius.density = item.value("density", 1.0);

		if (!cityType.sizeDefs.empty() && radius.population > cityType.sizeDefs.back().population) {
			Log::Warning("\tSize definition {} has higher population threshold than previous ({}b vs. {}b).\n" \
				"Size defs should be in descending order.",
				numSizeDefs, radius.population, cityType.sizeDefs.back().population);
		}

		cityType.sizeDefs.emplace_back(std::move(radius));
	}

	std::sort(cityType.sizeDefs.begin(), cityType.sizeDefs.end(), [](auto &a, auto &b) -> bool {
		return a.population > b.population;
	});

	Log::Verbose("\tLoaded {} city size definitions", numSizeDefs);

	uint32_t numBuildingDefs = 0;

	for (const auto &iter : buildingDefs.items()) {
		BuildingType building = {};
		numBuildingDefs++;

		try {
			LoadBuildingType(iter.key(), iter.value(), building);
		} catch (std::exception &e) {
			Log::Warning("Couldn't parse building def {}; error: {}", numBuildingDefs, e.what());
			continue;
		}

		cityType.buildingTypes.emplace_back(std::move(building));
	}

	Log::Verbose("\tLoaded {} building definitions", numBuildingDefs);

	s_cityFlavours.emplace_back(std::move(cityType));

	Log::Info("\tCreated city definition flavour #{}", s_cityFlavours.size());
}

void CityOnPlanet::Init()
{
	PROFILE_SCOPED()

	// Load all city definition configs
	FileSystem::FileEnumerator iter(FileSystem::gameDataFiles, "configs/buildings/");
	for (; !iter.Finished(); iter.Next()) {
		const FileSystem::FileInfo &file = iter.Current();
		std::string name = file.GetName();

		if (!ends_with_ci(name, ".json")) {
			continue;
		}

		try {
			LoadCityFlavour(file);
		} catch (std::exception &e) {
			Log::Warning("CityOnPlanet: failed to parse city definition '{}': {}", name, e.what());
		}
	}

	if (s_cityFlavours.empty()) {
		Log::Error("CityOnPlanet: failed to load any city definitions, no cities will be generated!");
	}

	Graphics::RenderStateDesc rsd = {};
	rsd.depthTest = false;
	rsd.depthWrite = false;
	rsd.primitiveType = Graphics::LINE_SINGLE;

	s_debugMat.reset(Pi::renderer->CreateMaterial("vtxColor", Graphics::MaterialDescriptor(), rsd));
}

void CityOnPlanet::Uninit()
{
	s_cityFlavours.clear();
	s_debugMat.reset();
}

// FIXME: this is a horrible idea as it directly mutates models in the ModelCache
// and forces all instances of a building to have the exact same color.
// Color data should be supplied via an instance buffer and managed as part of a building instance.
//static
void CityOnPlanet::SetCityModelPatterns(const SystemPath &path)
{
	PROFILE_SCOPED()
	// FIXME: should use system seed + body seed instead of path index
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);

	SceneGraph::ModelSkin skin;
	for (auto &cityFlavour : s_cityFlavours) {
		for (auto &buildingType : cityFlavour.buildingTypes) {

			SceneGraph::Model *model = buildingType.model;

			if (!model->SupportsPatterns())
				continue;

			skin.SetRandomColors(rand);
			skin.Apply(model);

			model->SetPattern(rand.Int32(model->GetNumPatterns()));
		}
	}
}

CityOnPlanet::~CityOnPlanet()
{
	// frame may be null (already removed from
	for (unsigned int i = 0; i < m_buildings.size(); i++) {
		Frame *f = Frame::GetFrame(m_frame);
		f->RemoveStaticGeom(m_buildings[i].geom);
		delete m_buildings[i].geom;
	}
}

CityOnPlanet::CityOnPlanet(Planet *planet, SpaceStation *station, const Uint32 seed)
{
	// beware, these are not used in this function, but are used in subroutines!
	m_body = planet->GetSystemBody();
	m_planet = planet;
	m_frame = planet->GetFrame();

	m_rand.seed(seed);

	if (s_cityFlavours.empty()) {
		return; // no buildings available to generate, we already logged an error about this during startup
	}

	// TODO: allow specifying city flavors based on various parameters (faction, world type, set from custom system def, etc.)
	m_cityType = &s_cityFlavours[m_rand.Int32(s_cityFlavours.size())];

	CalcCityRadius(m_body);

	// TODO: run in a task thread
	Generate(station);

	AddStaticGeomsToCollisionSpace();
}

void CityOnPlanet::Generate(SpaceStation *station)
{
	PROFILE_SCOPED()
	Profiler::Clock _genTimer;
	_genTimer.Start();

	// Create the acceleration grid structure
	// ==========================================

	// Retrieve the station parameters
	uint8_t stationSize[2] = {};
	GetModelSize(station->GetAabb(), stationSize);

	uint32_t stationSizeMax = std::max<uint32_t>(stationSize[0] / 2, stationSize[1] / 2);

	// update the allowed radius to ensure the spaceport isn't larger than the entire city
	m_cityRadius += stationSizeMax * CELLSIZE;

	// For now, just assume the city is a circle around the spaceport in the center
	const uint32_t cityExtents = (m_cityRadius / CELLSIZE);
	m_citySize = cityExtents * 2;
	m_gridPitch = std::ceil(m_citySize / 8.0);

	Log::Verbose("Generating City for spacestation {}", station->GetSystemBody()->GetName());
	Log::Verbose("\tpopulation: {0} size {1}x{1}c (radius {2})",
		m_body->GetPopulation(), m_citySize, m_cityRadius);


	// reserve space off the 'edge' of the grid for fast 64-bit lookup
	// prevents reading or writing unallocated memory on grid edges
	m_gridPitch = (m_gridPitch + 7) & ~uint32_t(7);

	m_gridLen = m_gridPitch * m_citySize;

	m_gridBitset.reset(new uint8_t[m_gridLen]);
	std::memset(m_gridBitset.get(), 0, m_gridLen);

	// Setup the station model position relative to the city grid
	// ==========================================

	// get the increment vectors to multiply X/Z grid coordinates by
	vector3d incX = station->GetOrient().VectorX() * CELLSIZE;
	vector3d incZ = station->GetOrient().VectorZ() * CELLSIZE;

	// top-left corner of the grid is -X, -Z relative to station position at center
	// offset origin by half-grid to ensure grid centers are aligned with the station model
	m_gridOrigin = station->GetPosition() - incX * (cityExtents + 0.5) - incZ * (cityExtents + 0.5);

	// Setup the station somewhere in the city (defaults to center for now)
	const uint32_t stationPos[2] = {
		cityExtents - stationSize[0] / 2,
		cityExtents - stationSize[1] / 2
	};

	// Reserve space for the spacestation
	SetGridOccupancy(stationPos[0], stationPos[1], stationSize);

	Log::Verbose("\tCityOnPlanet: Station {} placed at grid {}:{} with extents {}x{}",
		station->GetModel()->GetName(), stationPos[0], stationPos[1], stationSize[0], stationSize[1]);


	// Begin generating buildings for the city
	// ==========================================

	// precalc orientation transforms (to rotate buildings to face north/south/east/west)
	const matrix4x4d &m = station->GetOrient();

	matrix4x4d orientcalc[4];
	orientcalc[0] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 0);
	orientcalc[1] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 1);
	orientcalc[2] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 2);
	orientcalc[3] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 3);

	// Reserve space for all buildings we expect to generate
	m_buildings.clear();
	m_buildings.reserve(cityExtents * cityExtents); // estimate 25% occupancy

	bool hasAtmo = m_body->HasAtmosphere();

	uint32_t discardedCells = 0;
	uint32_t skippedCells = 0;
	uint32_t occupiedCells = 0;
	uint32_t underwaterCells = 0;
	uint32_t rarityRolls = 0;

	double cityRadiusSqr = m_cityRadius * m_cityRadius;

	for (uint32_t y = 0; y < m_citySize; y++) {
		for (uint32_t x = 0; x < m_citySize; x++) {

			// Do a basic loop over every cell in the city and determine what could be placed there

			// TODO: future versions of cities should generate a random road network or place individual
			// important buildings and place additional buildings around said features.
			// The quality of the result relative to the generation cost would be much improved.

			double radiusSqr = vector2d(
				(cityExtents - double(x) + 0.5) * CELLSIZE,
				(cityExtents - double(y) + 0.5) * CELLSIZE
			).LengthSqr();

			// skip cells outside the city radius
			if (radiusSqr >= cityRadiusSqr) {
				skippedCells++;
				continue;
			}

			// skip already full cells
			if (TestGridQuick(x, y)) {
				occupiedCells++;
				continue;
			}

			// allow the population of cells to drop off as distance from city center increases
			// note: this should ideally sample e.g. perlin noise to generate "clusters" of city buildings
			double radiusNorm = sqrt(radiusSqr) / m_cityRadius;
			if (radiusNorm > abs(m_rand.Normal()) * m_cityDensity) {
				discardedCells++;
				continue;
			}

			// Attempt to select a building type for this grid cell
			const BuildingType *buildingType = nullptr;
			uint32_t typeIndex = 0;
			for (uint32_t retry = 0; retry < 8; retry++) {

				// pick a random building type
				typeIndex = m_rand.Int32(m_cityType->buildingTypes.size());
				buildingType = &m_cityType->buildingTypes[typeIndex];

				// skip it if it won't fit here
				// FIXME: this does not correctly handle the rotation case for non-square buildings.
				// Need to transpose the cell size and test as well.
				if (TestGridOccupancy(x, y, buildingType->cellSize)) {
					buildingType = nullptr;
					continue;
				}

				float rarity = hasAtmo ? buildingType->rarityAtmo : buildingType->rarityAirless;
				float distribution = radiusNorm; // 0.0 .. 1.0 at edge of city

				if (buildingType->buildingKind == SectorKind::Storage) {
					// rarer in the center and more present at the edges of the city
					rarity *= 0.5 + distribution * 0.5;
				} else if (buildingType->buildingKind == SectorKind::Industry) {
					// rarer in the center and more present at the edges of the city
					rarity *= 0.3 + distribution * 0.7;
				} else if (buildingType->buildingKind == SectorKind::Monument) {
					// prevalent in the city center but extremely rare on the outskirts
					rarity *= (1.0 - distribution) * 0.5;
				} else if (buildingType->buildingKind == SectorKind::Habitat) {
					// 1.0 at city center and 0.2 at outskirts
					rarity *= 1.0 - distribution * 0.8;
				} else if (buildingType->buildingKind == SectorKind::Frontier) {
					// extremely rare at city center and 1.5 at outskirts
					rarity *= (distribution * distribution) * 1.5;
				}

				// roll to see if the building spawns here or not
				if (rarity < 1.0 && m_rand.Double() > rarity) {
					rarityRolls++;
					buildingType = nullptr;
					continue;
				}

			}

			// if we cannot find a building type, leave the cell empty and move on
			if (buildingType == nullptr) {
				discardedCells++;
				continue;
			}

			SetGridOccupancy(x, y, buildingType->cellSize);

			// rotate the building to face a random direction
			const int32_t orient = m_rand.Int32(4);

			// TODO: position model inside cells, add to building list
			vector2d buildingPos = {
				double(x) + buildingType->cellSize[0] / 2.0,
				double(y) + buildingType->cellSize[1] / 2.0,
			};

			vector3d pos = m_gridOrigin + incX * buildingPos.x + incZ * buildingPos.y;
			vector3d posNorm = pos.Normalized();
			double height = m_planet->GetTerrainHeight(posNorm);

			// don't place under planetary sea-level if the body has >10% water
			// TODO: need a better way to sample both height and biome data to determine if the cell is actually water
			// This will not properly handle elevated lakes or dry inland depressions below sea-level
			if (m_body->GetVolatileLiquid() > 0.1 && height < m_body->GetRadius()) {
				underwaterCells++;
				continue;
			}

			// Compute the terrain relative height by scaling the normal of the building's ideal position
			// This may introduce horizontal inaccuracy errors with sufficiently small planetary radii
			pos = posNorm * height;

			const CollMesh *cmesh = buildingType->model->GetCollisionMesh().Get();

			// FIXME: geoms need a userdata to tell gameplay code what we actually hit.
			// We don't want to create a separate Body for each instance of the buildings, so we
			// scam the code by pretending we're part of the host planet.
			Geom *geom = new Geom(cmesh->GetGeomTree(), orientcalc[orient], pos, GetPlanet());

			// add it to the list of buildings to render
			m_buildings.push_back({ typeIndex, float(cmesh->GetRadius()), orient, pos, geom });

		}
	}

	_genTimer.Stop();

	Log::Verbose("\tCityOnPlanet: generated {} buildings in {}ms ( {} skipped, {} discarded, {} occupied, {} underwater, {} avg rolls / building )",
		m_buildings.size(), _genTimer.milliseconds(), skippedCells, discardedCells, occupiedCells, underwaterCells, rarityRolls / double(m_buildings.size()));

	_genTimer.SoftReset();

	// Compute the total AABB of this city
	Aabb totalAABB;
	const vector3d &stationOrigin = station->GetPosition();

	for (const auto &buildingInst : m_buildings) {
		totalAABB.Update(buildingInst.pos - stationOrigin);
	}

	m_realCentre = totalAABB.min + ((totalAABB.max - totalAABB.min) * 0.5);
	m_clipRadius = totalAABB.GetRadius();

	_genTimer.Stop();

	Log::Verbose("\tCityOnPlanet: generated AABB for city in {}ms", _genTimer.milliseconds());

	// Release the memory once we're done generating and reset
	m_gridBitset.reset();
	m_gridPitch = 0;
	m_gridLen = 0;
}

// Calculate the radius for this city based on the SystemBody's parameters
void CityOnPlanet::CalcCityRadius(const SystemBody *body)
{
	double population = body->GetPopulation();
	double atmo = body->GetAtmosOxidizing();

	for (const CityRadiusDef &radii : reverse_container(m_cityType->sizeDefs)) {
		bool isLast = (&radii == &m_cityType->sizeDefs.front());

		if (population > radii.population && !isLast)
			continue;

		m_cityRadius = radii.baseSize + (atmo * radii.atmoSize) + (m_rand.Double() * radii.randomSize);
		m_cityDensity = radii.density * 0.8;
		break;
	}
}

#ifdef __BIG_ENDIAN__
#error "CityOnPlanet does not (yet) support big-endian architectures"
#endif

// Supports up to 32-cell wide buildings to ensure 8-byte aligned addressing in a single operation
static inline uint64_t CalcBitmaskForGrid(uint32_t x, uint32_t xsize)
{
	uint32_t maskOffset = x & CityOnPlanet::CELLMASK;
	uint64_t bitmask = uint64_t((1 << xsize) - 1) << maskOffset;
	// note: need crossplatform bswap_64 here for big-endian compat
	// we should be able to swap the bitmask rather than the in-memory bytes and have the same effect

	return bitmask;
}

void CityOnPlanet::SetGridOccupancy(uint32_t x, uint32_t y, const uint8_t size[2])
{
	if (x + size[0] > m_citySize || y + size[1] > m_citySize)
		return; // footprint would be off-grid, prevent writing outside the bitset

	// City grid is stored as lsb-first bitset with one bit per grid cell;
	// bit 1 of byte 1 is grid 0, while bit 8 of byte 4 is grid 31
	// Support maximum size of 32 cells for a single building to enable fast 64-bit aligned operations

	uint8_t *dataPtr = m_gridBitset.get() + (x & ~CELLMASK) / 8;
	uint64_t bitmask = CalcBitmaskForGrid(x, size[0]);

	for (uint32_t idx = y; idx < y + size[1]; idx++) {
		*reinterpret_cast<uint64_t *>(dataPtr + idx * m_gridPitch) |= bitmask;
	}
}

bool CityOnPlanet::TestGridOccupancy(uint32_t x, uint32_t y, const uint8_t size[2])
{
	if (x + size[0] > m_citySize || y + size[1] > m_citySize)
		return true; // always occupied if footprint would be off-grid


	// City grid is stored as lsb-first bitset with one bit per grid cell;
	// bit 1 of byte 1 is grid 0, while bit 8 of byte 4 is grid 31
	// Support maximum size of 32 cells for a single building to enable fast 64-bit aligned operations
	uint8_t *dataPtr = m_gridBitset.get() + (x & ~CELLMASK) / 8;
	uint64_t bitmask = CalcBitmaskForGrid(x, size[0]);

	uint64_t test = 0;

	for (uint32_t idx = y; idx < y + size[1]; idx++) {
		test |= *reinterpret_cast<uint64_t *>(dataPtr + idx * m_gridPitch) & bitmask;
	}

	return test != 0;
}

void CityOnPlanet::Render(Graphics::Renderer *r, const Graphics::Frustum &frustum, const SpaceStation *station, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	PROFILE_SCOPED()

	// Early out for failure case
	if (!m_cityType)
		return;

	// Early frustum test of whole city.
	const vector3d stationPos = viewTransform * (station->GetPosition() + m_realCentre);
	//modelview seems to be always identity
	if (!frustum.TestPoint(stationPos, m_clipRadius))
		return;

	// Precalculate building orientation matrices
	matrix4x4d rot[4];
	matrix4x4f rotf[4];
	rot[0] = station->GetOrient();

	// change detail level if necessary
	const bool bDetailChanged = m_detailLevel != Pi::detail.cities;
	if (bDetailChanged) {
		RemoveStaticGeomsFromCollisionSpace();
		AddStaticGeomsToCollisionSpace();
	}

	rot[0] = viewTransform * rot[0];
	for (int i = 1; i < 4; i++) {
		rot[i] = rot[0] * matrix4x4d::RotateYMatrix(M_PI * 0.5 * double(i));
	}
	for (int i = 0; i < 4; i++) {
		for (int e = 0; e < 16; e++) {
			rotf[i][e] = float(rot[i][e]);
		}
	}

	// update any idle animations
	// TODO: this is kind of a horrible idea in many ways
	// animated buildings need per-instance state and need to support instanced drawing with GPU animation transform of some kind
	// individual cities at the *very* least need to have their own copy of the model used in them so color + animation state
	// doesn't "leak" into other users of the model
	for (auto &buildingType : m_cityType->buildingTypes) {
		SceneGraph::Animation *anim = buildingType.idleAnimation;
		if (anim) {
			anim->SetProgress(fmod(anim->GetProgress() + (Pi::game->GetTimeStep() / anim->GetDuration()), 1.0));
			anim->Interpolate();
		}
	}

	uint32_t uCount = 0;
	uint32_t numBuildings = m_cityType->buildingTypes.size();

	std::vector<std::vector<matrix4x4f>> transform;

	transform.resize(numBuildings);

	for (uint32_t i = 0; i < numBuildings; i++) {
		transform[i].reserve(m_buildingCounts[i]);
	}

	for (const auto &building : m_enabledBuildings) {
		const vector3d pos = viewTransform * building.pos;

		if (!frustum.TestPoint(pos, building.clipRadius))
			continue;

		matrix4x4f instanceRot = matrix4x4f(rotf[building.rotation]);
		instanceRot.SetTranslate(vector3f(pos));

		transform[building.instIndex].push_back(instanceRot);
		++uCount;
	}

	// render the building models using instancing
	for (Uint32 i = 0; i < numBuildings; i++) {
		if (!transform[i].empty())
			m_cityType->buildingTypes[i].model->Render(transform[i]);
	}

	// Draw debug extents
	/*
	r->SetTransform(matrix4x4f(viewTransform));
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE);

	vector3f origin = vector3f(m_gridOrigin);
	vector3f incX = vector3f(station->GetOrient().VectorX() * double(m_citySize * CELLSIZE));
	vector3f incZ = vector3f(station->GetOrient().VectorZ() * double(m_citySize * CELLSIZE));

	va.Add(origin, Color(255, 0, 0));
	va.Add(origin + incX, Color(255, 0, 0));

	va.Add(origin + incX, Color(0, 255, 0));
	va.Add(origin + incX + incZ, Color(0, 255, 0));

	va.Add(origin + incX + incZ, Color(255, 0, 0));
	va.Add(origin + incZ, Color(255, 0, 0));

	va.Add(origin + incZ, Color(0, 255, 0));
	va.Add(origin, Color(0, 255, 0));

	r->DrawBuffer(&va, s_debugMat.get());
	*/

	r->GetStats().AddToStatCount(Graphics::Stats::STAT_BUILDINGS, uCount);
	r->GetStats().AddToStatCount(Graphics::Stats::STAT_CITIES, 1);
}
