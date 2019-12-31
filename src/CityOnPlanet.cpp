// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CityOnPlanet.h"

#include "FileSystem.h"
#include "Frame.h"
#include "Game.h"
#include "ModelCache.h"
#include "Pi.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "collider/Geom.h"
#include "graphics/Frustum.h"
#include "scenegraph/Animation.h"
#include "scenegraph/ModelSkin.h"
#include "scenegraph/SceneGraph.h"

static const unsigned int DEFAULT_NUM_BUILDINGS = 1000;
static const double START_SEG_SIZE = CITY_ON_PLANET_RADIUS;
static const double START_SEG_SIZE_NO_ATMO = CITY_ON_PLANET_RADIUS / 5.0f;

using SceneGraph::Model;

CityOnPlanet::citybuildinglist_t CityOnPlanet::s_buildingList = {
	"city_building",
	800,
	2000,
	0,
	0,
};

CityOnPlanet::cityflavourdef_t CityOnPlanet::cityflavour[CITYFLAVOURS];

void CityOnPlanet::AddStaticGeomsToCollisionSpace()
{
	// reset data structures
	m_enabledBuildings.clear();
	m_buildingCounts.resize(s_buildingList.numBuildings);
	for (Uint32 i = 0; i < s_buildingList.numBuildings; i++) {
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
	list->numBuildings = models.size();

	int i = 0;
	for (auto m = models.begin(), itEnd = models.end(); m != itEnd; ++m, i++) {
		list->buildings[i].instIndex = i;
		list->buildings[i].resolvedModel = *m;
		list->buildings[i].idle = (*m)->FindAnimation("idle");
		list->buildings[i].collMesh = (*m)->CreateCollisionMesh();
		const Aabb &aabb = list->buildings[i].collMesh->GetAabb();
		const double maxx = std::max(fabs(aabb.max.x), fabs(aabb.min.x));
		const double maxy = std::max(fabs(aabb.max.z), fabs(aabb.min.z));
		list->buildings[i].xzradius = sqrt(maxx * maxx + maxy * maxy);
		Output(" - %s: %f\n", (*m)->GetName().c_str(), list->buildings[i].xzradius);
	}
	Output("End of buildings.\n");
}

void CityOnPlanet::Init()
{
	PROFILE_SCOPED()
	/* Resolve city model numbers since it is a bit expensive */
	LookupBuildingListModels(&s_buildingList);
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

CityOnPlanet::CityOnPlanet(Planet *planet, SpaceStation *station, const Uint32 seed)
{
	// beware, these are not used in this function, but are used in subroutines!
	m_planet = planet;
	m_frame = planet->GetFrame();
	m_detailLevel = Pi::detail.cities;

	m_buildings.clear();
	m_buildings.reserve(DEFAULT_NUM_BUILDINGS);

	const Aabb &aabb = station->GetAabb();
	const matrix4x4d &m = station->GetOrient();
	const vector3d p = station->GetPosition();

	const vector3d mx = m * vector3d(1, 0, 0);
	const vector3d mz = m * vector3d(0, 0, 1);

	Random rand;
	rand.seed(seed);

	int population = planet->GetSystemBody()->GetPopulation();
	int cityradius;

	if (planet->GetSystemBody()->HasAtmosphere()) {
		population *= 1000;
		cityradius = (population < 200) ? 200 : ((population > START_SEG_SIZE) ? START_SEG_SIZE : population);
	} else {
		population *= 100;
		cityradius = (population < 250) ? 250 : ((population > START_SEG_SIZE_NO_ATMO) ? START_SEG_SIZE_NO_ATMO : population);
	}

	citybuildinglist_t *buildings = &s_buildingList;
	vector3d cent = p;
	const int cellsize_i = 80;
	const double cellsize = double(cellsize_i); // current widest building = 92
	const double bodyradius = planet->GetSystemBody()->GetRadius(); // cache for bodyradius value

	static const int gmid = (cityradius / cellsize_i);
	static const int gsize = gmid * 2;

	assert((START_SEG_SIZE / cellsize_i) < 100);
	assert((START_SEG_SIZE_NO_ATMO / cellsize_i) < 100);
	uint8_t cellgrid[200][200];
	std::memset(cellgrid, 0, sizeof(cellgrid));

	// calculate the size of the station model
	const int x1 = floor(aabb.min.x / cellsize);
	const int x2 = ceil(aabb.max.x / cellsize);
	const int z1 = floor(aabb.min.z / cellsize);
	const int z2 = ceil(aabb.max.z / cellsize);

	// Clear the cells where the station is
	for (int x = 0; x <= gsize; x++) {
		for (int z = 0; z <= gsize; z++) {
			const int zz = z - gmid;
			const int xx = x - gmid;
			if (zz > z1 && zz < z2 && xx > x1 && xx < x2)
				cellgrid[x][z] = 1;
		}
	}

	// precalc orientation transforms (to rotate buildings to face north/south/east/west)
	matrix4x4d orientcalc[4];
	orientcalc[0] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 0);
	orientcalc[1] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 1);
	orientcalc[2] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 2);
	orientcalc[3] = m * matrix4x4d::RotateYMatrix(M_PI * 0.5 * 3);

	const double maxdist = pow(gmid + 0.333, 2);
	for (int x = 0; x <= gsize; x++) {
		const double distx = pow((x - gmid), 2);
		for (int z = 0; z <= gsize; z++) {
			if (cellgrid[x][z] > 0) {
				// This cell has been allocated for something already
				continue;
			}
			const double distz = pow((z - gmid), 2);
			if ((distz + distx) > maxdist)
				continue;

			// fewer and fewer buildings the further from center you get
			if ((distx + distz) * (1.0 / maxdist) > rand.Double())
				continue;

			cent = p + mz * ((z - gmid) * cellsize) + mx * ((x - gmid) * cellsize);
			cent = cent.Normalized();

			const double height = planet->GetTerrainHeight(cent);
			if ((height - bodyradius) < 0) // don't position below sealevel
				continue;

			cent = cent * height;

			// quickly get a random building
			const citybuilding_t &bt = buildings->buildings[rand.Int32(buildings->numBuildings)];
			const CollMesh *cmesh = bt.collMesh.Get(); // collision mesh

			// rotate the building to face a random direction
			const int32_t orient = rand.Int32(4);
			Geom *geom = new Geom(cmesh->GetGeomTree(), orientcalc[orient], cent, this);

			// add it to the list of buildings to render
			m_buildings.push_back({ bt.instIndex, float(cmesh->GetRadius()), orient, cent, geom });
		}
	}

	Aabb buildAABB;
	for (std::vector<BuildingDef>::const_iterator iter = m_buildings.begin(), itEND = m_buildings.end(); iter != itEND; ++iter) {
		buildAABB.Update((*iter).pos - p);
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
		SceneGraph::Animation *pAnim = s_buildingList.buildings[i].idle;
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
