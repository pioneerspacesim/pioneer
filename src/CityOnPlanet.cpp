// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
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
#include "graphics/Frustum.h"
#include "scenegraph/Animation.h"
#include "scenegraph/ModelSkin.h"
#include "scenegraph/SceneGraph.h"

#define DEBUG_OUTPUT 1

static const char *CITY_CONFIG_FILE = "models/buildings/details.json";
static uint8_t citydistancegrid[CityOnPlanet::citygridlimit][CityOnPlanet::citygridlimit];
static double population2radius[5];
static double cityradius_mod[5];
static uint32_t cityradius_rnd[5];

int simplebuildings[50];
int numberofSimplebuildings;

static const uint32_t DEFAULT_ATMO_SIZE;
static const uint32_t DEFAULT_AIRLESS_SIZE;
uint32_t config_atmo_size;
uint32_t config_airless_size;

using SceneGraph::Model;

CityOnPlanet::citybuildinglist_t CityOnPlanet::s_buildingList = {
	"city_building",
	0,
	0,
};

void CityOnPlanet::AddStaticGeomsToCollisionSpace()
{
	// reset data structures
	m_enabledBuildings.clear();
	m_buildingCounts.resize(s_buildingList.numBuildings);
	for (uint32_t i = 0; i < s_buildingList.numBuildings; i++) {
		m_buildingCounts[i] = 0;
	}

	// Generate the new building list
	int skipMask;
	if (Pi::detail.cities >= 0 && Pi::detail.cities <= 3)
		skipMask = (16 >> Pi::detail.cities) - 1;
	else
		skipMask = 0;

	uint32_t numVisibleBuildings = 0;
	for (uint32_t i = 0; i < m_buildings.size(); i++) {
		if (!(i & skipMask)) {
			++numVisibleBuildings;
		}
	}

	// we know how many building we'll be adding, reserve space up front
	m_enabledBuildings.reserve(numVisibleBuildings);
	for (int i = 0; i < m_buildings.size(); i++) {
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

// Get all model file names under buildings/
// This is temporary. Buildings should be defined in BuildingSet data files, or something.
//static
void CityOnPlanet::EnumerateNewBuildings(std::set<std::string> &filenames)
{
	const std::string fullpath = FileSystem::JoinPathBelow("models", "buildings");
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, fullpath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const std::string &name = files.Current().GetName();
		if (ends_with_ci(name, ".model")) {
			filenames.insert(name.substr(0, name.length() - 6));
		} else if (ends_with_ci(name, ".sgm")) {
			filenames.insert(name.substr(0, name.length() - 4));
		}
	}
}

//static
void CityOnPlanet::LookupBuildingListModels(citybuildinglist_t *list)
{
	std::vector<Model *> models;

	//get test newmodels - to be replaced with building set definitions
	{
		std::set<std::string> filenames; // set so we get unique names
		EnumerateNewBuildings(filenames);
		for (auto it = filenames.begin(), itEnd = filenames.end(); it != itEnd; ++it) {
			// find/load the model
			Model *model = Pi::modelCache->FindModel(*it);

			// good to use
			models.push_back(model);
		}
	}
	assert(!models.empty());
	Output("Got %d buildings of tag %s\n", static_cast<int>(models.size()), list->modelTagName);
	list->buildings = new citybuilding_t[models.size()];
	list->numBuildings = (uint32_t)models.size();
	numberofSimplebuildings = 0;

	Json data = JsonUtils::LoadJsonDataFile(CITY_CONFIG_FILE);

	// allow config to determine city sizes
	config_atmo_size = DEFAULT_ATMO_SIZE;
	config_airless_size = DEFAULT_AIRLESS_SIZE;
	if (!data["global"].is_null()) {
		config_atmo_size = data["global"].value("atmo-size", DEFAULT_ATMO_SIZE);
		config_airless_size = data["global"].value("airless-size", DEFAULT_AIRLESS_SIZE);

		population2radius[0] = data["global"].value("pop-0", 1000000000.0);
		population2radius[1] = data["global"].value("pop-1", 200000000.0);
		population2radius[2] = data["global"].value("pop-2", 20000000.0);
		population2radius[3] = data["global"].value("pop-3", 1000000.0);
		population2radius[4] = 0.0;

		cityradius_mod[0] = data["global"].value("mod-0", 1.0);
		cityradius_mod[1] = data["global"].value("mod-1", 0.8);
		cityradius_mod[2] = data["global"].value("mod-2", 0.6);
		cityradius_mod[3] = data["global"].value("mod-3", 0.4);
		cityradius_mod[4] = data["global"].value("mod-4", 0.2);

		cityradius_rnd[0] = data["global"].value("rnd-0", 30);
		cityradius_rnd[1] = data["global"].value("rnd-1", 15);
		cityradius_rnd[2] = data["global"].value("rnd-2", 14);
		cityradius_rnd[3] = data["global"].value("rnd-3", 12);
		cityradius_rnd[4] = data["global"].value("rnd-4", 10);
	}
	if (config_atmo_size > 68 || config_atmo_size < 5)
		config_atmo_size = DEFAULT_ATMO_SIZE;
	if (config_airless_size > 68 || config_airless_size < 5)
		config_airless_size = DEFAULT_AIRLESS_SIZE;

	int i = 0;
	for (auto m = models.begin(); m != models.end(); ++m, i++) {
		list->buildings[i].index = i;
		list->buildings[i].layout = 0;
		list->buildings[i].x_offset = 0;
		list->buildings[i].z_offset = 0;
		list->buildings[i].airless_rarity = 1.0;
		list->buildings[i].atmo_rarity = 1.0;
		list->buildings[i].storage = false;
		list->buildings[i].industry = false;
		list->buildings[i].monument = false;
		list->buildings[i].habitat = true;
		list->buildings[i].resolvedModel = *m;
		list->buildings[i].idleanimation = (*m)->FindAnimation("idle");
		list->buildings[i].collMesh = (*m)->CreateCollisionMesh();

		const std::string modelname = (*m)->GetName();
		std::string extra_attributes = "";
		if (!data[modelname].is_null()) {
			extra_attributes = "extras:";
			list->buildings[i].layout = data[modelname].value("layout", 0);
			if (list->buildings[i].layout != 0) extra_attributes += " layout";
			list->buildings[i].x_offset = data[modelname].value("x-offset", 0);
			if (list->buildings[i].x_offset != 0) extra_attributes += " x-offset";
			list->buildings[i].z_offset = data[modelname].value("z-offset", 0);
			if (list->buildings[i].z_offset != 0) extra_attributes += " z-offset";
			list->buildings[i].airless_rarity = data[modelname].value("airless-rarity", 1.0);
			if (list->buildings[i].airless_rarity != 1.0) extra_attributes += " airless-rarity";
			list->buildings[i].atmo_rarity = data[modelname].value("atmospheric-rarity", 1.0);
			if (list->buildings[i].atmo_rarity != 1.0) extra_attributes += " atmospheric-rarity";
			list->buildings[i].storage = data[modelname].value("storage", false);
			if (list->buildings[i].storage != false) extra_attributes += " storage";
			list->buildings[i].industry = data[modelname].value("industry", false);
			if (list->buildings[i].industry != false) extra_attributes += " industry";
			list->buildings[i].monument = data[modelname].value("monument", false);
			if (list->buildings[i].monument != false) extra_attributes += " monument";
			list->buildings[i].habitat = data[modelname].value("habitat", true);
			if (list->buildings[i].habitat != true) extra_attributes += " habitat";

			if (list->buildings[i].industry == true)
				list->buildings[i].habitat = false;
		}
		// keep a list of simple buildings
		assert(numberofSimplebuildings < 50);
		if (list->buildings[i].layout == 0 && list->buildings[i].storage == false && list->buildings[i].industry == false && list->buildings[i].monument == false	&& list->buildings[i].habitat == true && list->buildings[i].atmo_rarity == 1.0 && list->buildings[i].airless_rarity == 1.0) {
			simplebuildings[numberofSimplebuildings++] = i;
		}
		Output(" - %s: %s airless %.3f atmo %.3f\n", modelname, extra_attributes, list->buildings[i].airless_rarity, list->buildings[i].atmo_rarity);
	}
	Output("End of buildings.\n");
}

void CityOnPlanet::Init()
{
	PROFILE_SCOPED()
	/* Resolve city model numbers since it is a bit expensive */
	LookupBuildingListModels(&s_buildingList);

	assert(citygridlimit < 256);
	assert(citygridmidpoint < 128);

	// precalc each cells distance from the center
	for (int x = 0; x < citygridlimit; x++) {
		for (int z = 0; z < citygridlimit; z++) {
			citydistancegrid[x][z] = round(vector2d(x - citygridmidpoint, z - citygridmidpoint).Length());
		}
	}
}

void CityOnPlanet::Uninit()
{
	delete[] s_buildingList.buildings;
}

// Need a reliable way to sort the models rather than using their address in memory we use their name which should be unique.
bool setcomp(SceneGraph::Model *mlhs, SceneGraph::Model *mrhs) { return mlhs->GetName() < mrhs->GetName(); }
bool (*fn_pt)(SceneGraph::Model *mlhs, SceneGraph::Model *mrhs) = setcomp;

struct ModelNameComparator {
	bool operator()(const SceneGraph::Model *lhs, const SceneGraph::Model *rhs) const
	{
		return lhs->GetName() < rhs->GetName();
	}
};

//static
void CityOnPlanet::SetCityModelPatterns(const SystemPath &path)
{
	PROFILE_SCOPED()
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);

	typedef std::set<SceneGraph::Model *, ModelNameComparator> ModelSet;
	typedef ModelSet::iterator TSetIter;
	ModelSet modelSet;
	{
		for (unsigned int j = 0; j < s_buildingList.numBuildings; j++) {
			SceneGraph::Model *m = s_buildingList.buildings[j].resolvedModel;
			modelSet.insert(m);
		}
	}

	SceneGraph::ModelSkin skin;
	for (TSetIter it = modelSet.begin(), itEnd = modelSet.end(); it != itEnd; ++it) {
		SceneGraph::Model *m = (*it);
		if (!m->SupportsPatterns()) continue;
		skin.SetRandomColors(rand);
		skin.Apply(m);
		if (m->SupportsPatterns())
			m->SetPattern(rand.Int32(0, m->GetNumPatterns() - 1));
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

bool CityOnPlanet::isCityCellsOccupied(uint32_t x, uint32_t z, uint32_t layout)
{
	uint8_t *cells;
	uint16_t testbits = ((uint16_t)(0xff00 << (7 - layout))) >> (z & 7);
	uint16_t bits;

	for (uint32_t i = 0; i <= layout; i++) {
		cells = &citybits[x + i][z >> 3];
		bits = ((uint16_t)cells[0] << 8) | cells[1];
		if ((bits & testbits) != 0)
			return (true);
	}
	return (false);
}

void CityOnPlanet::setCityCellsOccupied(uint32_t x, uint32_t z, uint32_t layout)
{
	union bits {
		uint16_t whole;
		uint8_t bytes[2];
	} bits;

	bits.whole = ((uint16_t)(0xff00 << (7 - layout))) >> (z & 7);

	for (uint32_t i = 0; i <= layout; i++) {
#ifdef __BIG_ENDIAN__
		citybits[x + i][z >> 3] |= bits.bytes[0];
		citybits[x + i][(z >> 3) + 1] |= bits.bytes[1];
#else
		// x86/x64
		citybits[x + i][z >> 3] |= bits.bytes[1];
		citybits[x + i][(z >> 3) + 1] |= bits.bytes[0];
#endif
	}
}

CityOnPlanet::CityOnPlanet(Planet *planet, SpaceStation *station, const Uint32 seed)
{
	// beware, these are not used in this function, but are used in subroutines!
	m_planet = planet;
	m_frame = planet->GetFrame();
	m_detailLevel = Pi::detail.cities;

	// Group outcalls together to make things easier for cpu cache
	const Aabb &aabb = station->GetAabb();
	const matrix4x4d &m = station->GetOrient();
	const vector3d station_position = station->GetPosition();
	const std::string &stationmodelname = station->GetStationType()->ModelName();
	const double bodyradius = planet->GetSystemBody()->GetRadius();			// cache for bodyradius value
	const double pop = 1000000000 * planet->GetSystemBody()->GetPopulation(); // GetPopulation() returns population in whole billions, as in 0.5 -> 500M
	const int atmo = planet->GetSystemBody()->HasAtmosphere() ? config_atmo_size : config_airless_size;

	Random rand;
	rand.seed(seed);

	const vector3d mx = m * vector3d(1, 0, 0);
	const vector3d mz = m * vector3d(0, 0, 1);
	citybuildinglist_t *buildings = &s_buildingList;

	const int cellsize_i = 50;
	const double cellsize = double(cellsize_i);
	int cityradius = 0;

	for (int i = 0; i < 5; i++) {
		if (pop > population2radius[i]) {
			cityradius = (atmo * cityradius_mod[i]) + rand.Int32(cityradius_rnd[i]);
			break;
		}
	}

	if (cityradius > 98)
		cityradius = 98;
	if (cityradius < 10)
		cityradius = 10;

	// Clear grid occupancy bitfield
	std::memset(citybits, 0, sizeof(citybits));

	// calculate the size of the station model
	const int station_x1 = floor(aabb.min.x / cellsize) + citygridmidpoint + 1;
	const int station_x2 = ceil(aabb.max.x / cellsize) + citygridmidpoint;
	const int station_z1 = floor(aabb.min.z / cellsize) + citygridmidpoint + 1;
	const int station_z2 = ceil(aabb.max.z / cellsize) + citygridmidpoint;

	if (stationmodelname == "ground_station") {
		assert(cellsize_i == 50); // this block is specifically tuned to cellsize 50.
		int z = (citygridmidpoint >> 3);
		citybits[citygridmidpoint + 0][z] |= (64 + 32 + 16 + 8 + 4 + 2 + 1);
		citybits[citygridmidpoint - 1][z] |= (64 + 32 + 16 + 8 + 4 + 2 + 1);
		citybits[citygridmidpoint + 1][z] |= (64 + 32 + 16 + 8 + 4 + 2 + 1);
		citybits[citygridmidpoint - 2][z] |= (64 + 32 + 16 + 8 + 4 + 2 + 1);
		citybits[citygridmidpoint + 2][z] |= (64 + 32 + 16 + 8 + 4 + 2 + 1);
		citybits[citygridmidpoint - 3][z] |= (32 + 16 + 8 + 4 + 2);
		citybits[citygridmidpoint + 3][z] |= (32 + 16 + 8 + 4 + 2);
	} else {
		// Occupy the cells where the station is
		for (int x = station_x1; x < station_x2; x++) {
			for (int z = station_z1; z < station_z2; z++) {
				citybits[x][z >> 3] |= 128 >> (z & 7);
			}
		}
	}

	// precalc orientation transforms (to rotate buildings to face north/south/east/west)
	matrix4x4d orientcalc[4];
	orientcalc[0] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 0);
	orientcalc[1] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 1);
	orientcalc[2] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 2);
	orientcalc[3] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 3);

	Output("CityOnPlanet: Station '%s' X %i .. %i, Z %i .. %i, pop %.0f cityradius %i\n",
		stationmodelname, station_x1, station_x2, station_z1, station_z2, pop, cityradius);

#ifdef DEBUG_OUTPUT
	int skipped = 0, placed = 0, emptycells = 0, occupiedcells = 0, loopcells = 0, rarityrolls = 0;
#endif

	m_buildings.clear();
	m_buildings.reserve(cityradius * cityradius); // estimate 25% occupancy (cityradius*cityradius*4*0.25) -> 4*0.25 == 1

	const int mincell = citygridmidpoint - cityradius;
	const int maxcell = citygridmidpoint + cityradius;
	double x_offset = 0, z_offset = 0;
	vector3d cent;

	// Loop through all the available cells to find suitable spots for buildings.
	for (int x = mincell; x <= maxcell; x++) {
		const vector3d cent_mx_cache = station_position + mx * ((x - citygridmidpoint) * cellsize);
		for (int z = mincell; z <= maxcell; z++) {
#ifdef DEBUG_OUTPUT
			loopcells++;
#endif
			// Using precalculated distances saves a bunch of floating point math.
			if (citydistancegrid[x][z] > cityradius) {
#ifdef DEBUG_OUTPUT
				skipped++;
#endif
				continue;
			}
			// check for cells that are already occupied
			if ((citybits[x][z >> 3] & (128 >> (z & 7)))) {
#ifdef DEBUG_OUTPUT
				occupiedcells++;
#endif
				continue;
			}

			// fewer and fewer buildings the further from center you get
			if ((((double)cityradius - citydistancegrid[x][z]) / (double)cityradius) < rand.Double()) {
#ifdef DEBUG_OUTPUT
				emptycells++;
#endif
				continue;
			}

			// quickly get a random building
			const citybuilding_t *chosenBuilding;
			for (uint32_t retry = 0; retry < 8; retry++) {
				chosenBuilding = &buildings->buildings[rand.Int32(buildings->numBuildings)];
				if (chosenBuilding->layout > 0 && isCityCellsOccupied(x, z, chosenBuilding->layout)) {
					chosenBuilding = NULL;
					continue;
				}
				// rarity modifiers (from details.json)
				float rarity = (atmo == config_atmo_size) ? chosenBuilding->atmo_rarity : chosenBuilding->airless_rarity;

				// industry buildings are rarer in the center and more common on the fringe
				if (chosenBuilding->industry) {
					// modify rarity instead of having a separate roll, otherwise we'll be in this loop a lot longer
					rarity = rarity * (0.5 + ((double)(citydistancegrid[x][z] > 1) / cityradius));
					// rarity = x0.5 at center up to x1.0 at edge

				}
				if (chosenBuilding->habitat) {
					rarity = rarity * (1.0 - ((double)(citydistancegrid[x][z] > 1) / cityradius));
					// rarity = x1.0 at center down to x0.5 at edge
				}

				if (rarity < 1.0 && rand.Double() > rarity) {
#ifdef DEBUG_OUTPUT
					rarityrolls++;
#endif
					chosenBuilding = NULL;
					continue;
				}
			}

			if (chosenBuilding == NULL) {
				if ((citybits[x][z >> 3] & (128 >> (z & 7))) == 0) {
					chosenBuilding = &buildings->buildings[simplebuildings[rand.Int32(numberofSimplebuildings)]];
				} else
					continue;
			}
			// building offset modifiers (from details.json)
			if (chosenBuilding->x_offset)
				x_offset = chosenBuilding->x_offset;
			if (chosenBuilding->z_offset)
				z_offset = chosenBuilding->z_offset;
			// multicell layout (from details.json)
			if (chosenBuilding->layout > 0) {
				setCityCellsOccupied(x, z, chosenBuilding->layout);
				x_offset += chosenBuilding->layout * cellsize / 2;
				z_offset += chosenBuilding->layout * cellsize / 2;
			}

			if (x_offset > 0 || z_offset > 0) {
				cent = station_position + mx * ((x - citygridmidpoint) * cellsize + x_offset) + mz * ((z - citygridmidpoint) * cellsize + z_offset);
				x_offset = z_offset = 0;
			} else {
				cent = cent_mx_cache + mz * ((z - citygridmidpoint) * cellsize);
			}
			cent = cent.Normalized();

			const double height = planet->GetTerrainHeight(cent);
			if ((height - bodyradius) < 0) // don't position below sealevel
				continue;

			cent = cent * height;

			const CollMesh *cmesh = chosenBuilding->collMesh.Get(); // collision mesh
			// rotate the building to face a random direction
			const int32_t orient = rand.Int32(4);
			// FIXME: geoms need a userdata to tell gameplay code what we actually hit.
			// We don't want to create a separate Body for each instance of the buildings, so we
			// scam the code by pretending we're part of the host planet.
			Geom *geom = new Geom(cmesh->GetGeomTree(), orientcalc[orient], cent, GetPlanet());

			// add it to the list of buildings to render
			m_buildings.push_back({ chosenBuilding->index, float(cmesh->GetRadius()), orient, cent, geom });
#ifdef DEBUG_OUTPUT
			placed++;
#endif
		}
	}

#ifdef DEBUG_OUTPUT
	// This is statistics for debug and profiling purposes
	Output("CityOnPlanet: skipped %i, placed %i, emptycells %i, occupiedcells %i, loopcells %i, rarityrolls %i, buildings reserved space: %i\n",
		skipped, placed, emptycells, occupiedcells, loopcells, rarityrolls, (cityradius * cityradius));
#endif

	Aabb buildAABB;
	for (std::vector<BuildingDef>::const_iterator iter = m_buildings.begin(), itEND = m_buildings.end(); iter != itEND; ++iter) {
		buildAABB.Update((*iter).pos - station_position);
	}
	m_realCentre = buildAABB.min + ((buildAABB.max - buildAABB.min) * 0.5);
	m_clipRadius = buildAABB.GetRadius();
	AddStaticGeomsToCollisionSpace();
}

void CityOnPlanet::Render(Graphics::Renderer *r, const Graphics::Frustum &frustum, const SpaceStation *station, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	// Early frustum test of whole city.
	const vector3d stationPos = viewTransform * (station->GetPosition() + m_realCentre);
	//modelview seems to be always identity
	if (!frustum.TestPoint(stationPos, m_clipRadius))
		return;

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
	for (Uint32 i = 0; i < s_buildingList.numBuildings; i++) {
		SceneGraph::Animation *pAnim = s_buildingList.buildings[i].idleanimation;
		if (pAnim) {
			pAnim->SetProgress(fmod(pAnim->GetProgress() + (Pi::game->GetTimeStep() / pAnim->GetDuration()), 1.0));
			pAnim->Interpolate();
		}
	}

	Uint32 uCount = 0;
	std::vector<Uint32> instCount;
	std::vector<std::vector<matrix4x4f>> transform;
	instCount.resize(s_buildingList.numBuildings);
	transform.resize(s_buildingList.numBuildings);
	memset(&instCount[0], 0, sizeof(Uint32) * s_buildingList.numBuildings);
	for (Uint32 i = 0; i < s_buildingList.numBuildings; i++) {
		transform[i].reserve(m_buildingCounts[i]);
	}

	for (std::vector<BuildingDef>::const_iterator iter = m_enabledBuildings.begin(), itEND = m_enabledBuildings.end(); iter != itEND; ++iter) {
		const vector3d pos = viewTransform * (*iter).pos;
		const vector3f posf(pos);
		if (!frustum.TestPoint(pos, (*iter).clipRadius))
			continue;

		matrix4x4f _rot(rotf[(*iter).rotation]);
		_rot.SetTranslate(posf);

		// increment the instance count and store the transform
		instCount[(*iter).instIndex]++;
		transform[(*iter).instIndex].push_back(_rot);

		++uCount;
	}

	// render the building models using instancing
	for (Uint32 i = 0; i < s_buildingList.numBuildings; i++) {
		if (!transform[i].empty())
			s_buildingList.buildings[i].resolvedModel->Render(transform[i]);
	}

	r->GetStats().AddToStatCount(Graphics::Stats::STAT_BUILDINGS, uCount);
	r->GetStats().AddToStatCount(Graphics::Stats::STAT_CITIES, 1);
}
