// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
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
#include "graphics/Graphics.h"
#include "scenegraph/Model.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/ModelSkin.h"
#include <set>

static const unsigned int DEFAULT_NUM_BUILDINGS = 1000;
static const double  START_SEG_SIZE = CITY_ON_PLANET_RADIUS;
static const double MIN_SEG_SIZE = 50.0;
static const unsigned int CITYFLAVOURS = 5;

using SceneGraph::Model;

bool s_cityBuildingsInitted = false;
struct citybuilding_t {
	const char *modelname;
	double xzradius;
	Model *resolvedModel;
	RefCountedPtr<CollMesh> collMesh;
};

struct citybuildinglist_t {
	const char *modelTagName;
	double minRadius, maxRadius;
	unsigned int numBuildings;
	citybuilding_t *buildings;
};

static citybuildinglist_t s_buildingLists[] = {
	{ "city_building", 800, 2000, 0, 0 },
	//{ "city_power", 100, 250, 0, 0 },
	//{ "city_starport_building", 300, 400, 0, 0 },
};

struct cityflavourdef_t {
	int buildingListIdx;
	vector3d center;
	double size;
} cityflavour[CITYFLAVOURS];

void CityOnPlanet::PutCityBit(Random &rand, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4)
{
	double rad = (p1-p2).Length()*0.5;
	Model *model(0);
	double modelRadXZ(0.0);
	const CollMesh *cmesh(0);
	vector3d cent = (p1+p2+p3+p4)*0.25;

	cityflavourdef_t *flavour(0);
	citybuildinglist_t *buildings(0);

	// pick a building flavour (city, windfarm, etc)
	for (unsigned int flv = 0; flv < CITYFLAVOURS; flv++) {
		flavour = &cityflavour[flv];
		buildings = &s_buildingLists[flavour->buildingListIdx];

		int tries;
		for (tries=20; tries--; ) {
			const citybuilding_t &bt = buildings->buildings[rand.Int32(buildings->numBuildings)];
			model = bt.resolvedModel;
			modelRadXZ = bt.xzradius;
			cmesh = bt.collMesh.Get();
			if (modelRadXZ < rad) break;
			if (tries == 0) return;
		}

		bool tooDistant = ((flavour->center - cent).Length()*(1.0/flavour->size) > rand.Double());
		if (!tooDistant) break;
		else flavour = 0;
	}

	if (flavour == 0) {
		if (rad > MIN_SEG_SIZE) goto always_divide;
		else return;
	}

	if (rad > modelRadXZ*2.0) {
always_divide:
		vector3d a = (p1+p2)*0.5;
		vector3d b = (p2+p3)*0.5;
		vector3d c = (p3+p4)*0.5;
		vector3d d = (p4+p1)*0.5;
		vector3d e = (p1+p2+p3+p4)*0.25;
		PutCityBit(rand, rot, p1, a, e, d);
		PutCityBit(rand, rot, a, p2, b, e);
		PutCityBit(rand, rot, e, b, p3, c);
		PutCityBit(rand, rot, d, e, c, p4);
	} else {
		cent = cent.Normalized();
		double height = m_planet->GetTerrainHeight(cent);
		/* don't position below sealevel! */
		if (height - m_planet->GetSystemBody()->GetRadius() <= 0.0) return;
		cent = cent * height;

		Geom *geom = new Geom(cmesh->GetGeomTree());
		int rotTimes90 = rand.Int32(4);
		matrix4x4d grot = rot * matrix4x4d::RotateYMatrix(M_PI*0.5*double(rotTimes90));
		geom->MoveTo(grot, cent);
		geom->SetUserData(this);
//		f->AddStaticGeom(geom);

		BuildingDef def = { model, float(cmesh->GetRadius()), rotTimes90, cent, geom };
		m_buildings.push_back(def);
	}
}

void CityOnPlanet::AddStaticGeomsToCollisionSpace()
{
	m_enabledBuildings.clear();
	int skipMask;
	switch (Pi::detail.cities) {
		case 0: skipMask = 0xf; break;
		case 1: skipMask = 0x7; break;
		case 2: skipMask = 0x3; break;
		case 3: skipMask = 0x1; break;
		default:
			skipMask = 0; break;
	}
	Uint32 numVisibleBuildings = 0;
	for (unsigned int i=0; i<m_buildings.size(); i++) {
		if (!(i&skipMask)) {
			++numVisibleBuildings;
		}
	}

	// we know how many building we'll be adding, reserve space up front
	m_enabledBuildings.reserve(numVisibleBuildings);
	for (unsigned int i=0; i<m_buildings.size(); i++) {
		if (i & skipMask) {
		} else {
			m_frame->AddStaticGeom(m_buildings[i].geom);
			m_enabledBuildings.push_back(m_buildings[i]);
		}
	}
	m_detailLevel = Pi::detail.cities;
}

void CityOnPlanet::RemoveStaticGeomsFromCollisionSpace()
{
	m_enabledBuildings.clear();
	for (unsigned int i=0; i<m_buildings.size(); i++) {
		m_frame->RemoveStaticGeom(m_buildings[i].geom);
	}
}

// Get all model file names under buildings/
// This is temporary. Buildings should be defined in BuildingSet data files, or something.
static void enumerateNewBuildings(std::vector<std::string> &filenames)
{
	const std::string fullpath = FileSystem::JoinPathBelow("models", "buildings");
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, fullpath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
		const std::string &name = files.Current().GetName();
		if (ends_with_ci(name, ".model")) {
			filenames.push_back(name.substr(0, name.length()-6));
		}
	}
}

static void lookupBuildingListModels(citybuildinglist_t *list)
{
	std::vector<Model*> models;

	//get test newmodels - to be replaced with building set definitions
	{
		std::vector<std::string> filenames;
		enumerateNewBuildings(filenames);
		for (std::vector<std::string>::const_iterator it = filenames.begin();
			it != filenames.end(); ++it)
		{
			Model *model = Pi::modelCache->FindModel(*it);
			models.push_back(model);
		}
	}
	assert(!models.empty());
	//Output("Got %d buildings of tag %s\n", models.size(), list->modelTagName);
	list->buildings = new citybuilding_t[models.size()];
	list->numBuildings = models.size();

	int i = 0;
	for (std::vector<Model*>::iterator m = models.begin(); m != models.end(); ++m, i++) {
		list->buildings[i].resolvedModel = *m;
		list->buildings[i].collMesh = (*m)->CreateCollisionMesh();
		const Aabb &aabb = list->buildings[i].collMesh->GetAabb();
		const double maxx = std::max(fabs(aabb.max.x), fabs(aabb.min.x));
		const double maxy = std::max(fabs(aabb.max.z), fabs(aabb.min.z));
		list->buildings[i].xzradius = sqrt(maxx*maxx + maxy*maxy);
		//Output("%s: %f\n", list->buildings[i].modelname, list->buildings[i].xzradius);
	}
}

void CityOnPlanet::Init()
{
	/* Resolve city model numbers since it is a bit expensive */
	if (!s_cityBuildingsInitted) {
		s_cityBuildingsInitted = true;
		for (unsigned int i=0; i<COUNTOF(s_buildingLists); i++) {
			lookupBuildingListModels(&s_buildingLists[i]);
		}
	}
}

void CityOnPlanet::Uninit()
{
	for (unsigned int list=0; list<COUNTOF(s_buildingLists); list++) {
		delete[] s_buildingLists[list].buildings;
	}
}

// Need a reliable way to sort the models rather than using there address in memory we use their name which should be unique.
bool setcomp (SceneGraph::Model *mlhs, SceneGraph::Model *mrhs) {return mlhs->GetName()<mrhs->GetName();}
bool(*fn_pt)(SceneGraph::Model *mlhs, SceneGraph::Model *mrhs) = setcomp;

struct ModelNameComparator {
	bool operator()(SceneGraph::Model* lhs, SceneGraph::Model* rhs) {
		return lhs->GetName() < rhs->GetName();
	}
};

//static
void CityOnPlanet::SetCityModelPatterns(const SystemPath &path)
{
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);

	typedef std::set<SceneGraph::Model*, ModelNameComparator> ModelSet;
	typedef ModelSet::iterator TSetIter;
	ModelSet modelSet;
	for (unsigned int i=0; i<COUNTOF(s_buildingLists); i++) {
		for (unsigned int j=0; j < s_buildingLists[i].numBuildings; j++) {
			SceneGraph::Model *m = s_buildingLists[i].buildings[j].resolvedModel;
			modelSet.insert(m);
		}
	}

	SceneGraph::ModelSkin skin;
	for (TSetIter it=modelSet.begin(), itEnd=modelSet.end(); it!=itEnd; ++it) {
		SceneGraph::Model *m = (*it);
		if (!m->SupportsPatterns()) continue;
		skin.SetRandomColors(rand);
		skin.Apply(m);
		m->SetPattern(rand.Int32(0, m->GetNumPatterns()));
	}
}

CityOnPlanet::~CityOnPlanet()
{
	// frame may be null (already removed from
	for (unsigned int i=0; i<m_buildings.size(); i++) {
		m_frame->RemoveStaticGeom(m_buildings[i].geom);
		delete m_buildings[i].geom;
	}
}

CityOnPlanet::CityOnPlanet(Planet *planet, SpaceStation *station, const Uint32 seed)
{
	m_buildings.clear();
	m_buildings.reserve(DEFAULT_NUM_BUILDINGS);
	m_planet = planet;
	m_frame = planet->GetFrame();
	m_detailLevel = Pi::detail.cities;

	/* Resolve city model numbers since it is a bit expensive */
	if (!s_cityBuildingsInitted) {
		s_cityBuildingsInitted = true;
		for (unsigned int i=0; i<COUNTOF(s_buildingLists); i++) {
			lookupBuildingListModels(&s_buildingLists[i]);
		}
	}

	const Aabb &aabb = station->GetAabb();
	matrix4x4d m = station->GetOrient();

	vector3d mx = m*vector3d(1,0,0);
	vector3d mz = m*vector3d(0,0,1);

	Random rand;
	rand.seed(seed);

	const vector3d p = station->GetPosition();

	vector3d p1, p2, p3, p4;
	double sizex = START_SEG_SIZE;// + rand.Int32((int)START_SEG_SIZE);
	double sizez = START_SEG_SIZE;// + rand.Int32((int)START_SEG_SIZE);

	// always have random shipyard buildings around the space station
	cityflavour[0].buildingListIdx = 0;//2;
	cityflavour[0].center = p;
	cityflavour[0].size = 500;

	for (unsigned int i = 1; i < CITYFLAVOURS; i++) {
		cityflavour[i].buildingListIdx =
			(COUNTOF(s_buildingLists) > 1 ? rand.Int32(COUNTOF(s_buildingLists)) : 0);
		citybuildinglist_t *blist = &s_buildingLists[cityflavour[i].buildingListIdx];
		double a = rand.Int32(-1000,1000);
		double b = rand.Int32(-1000,1000);
		cityflavour[i].center = p + a*mx + b*mz;
		cityflavour[i].size = rand.Int32(int(blist->minRadius), int(blist->maxRadius));
	}

	for (int side=0; side<4; side++) {
		/* put buildings on all sides of spaceport */
		switch(side) {
			case 3:
				p1 = p + mx*(aabb.min.x) + mz*aabb.min.z;
				p2 = p + mx*(aabb.min.x) + mz*(aabb.min.z-sizez);
				p3 = p + mx*(aabb.min.x+sizex) + mz*(aabb.min.z-sizez);
				p4 = p + mx*(aabb.min.x+sizex) + mz*(aabb.min.z);
				break;
			case 2:
				p1 = p + mx*(aabb.min.x-sizex) + mz*aabb.max.z;
				p2 = p + mx*(aabb.min.x-sizex) + mz*(aabb.max.z-sizez);
				p3 = p + mx*(aabb.min.x) + mz*(aabb.max.z-sizez);
				p4 = p + mx*(aabb.min.x) + mz*(aabb.max.z);
				break;
			case 1:
				p1 = p + mx*(aabb.max.x-sizex) + mz*aabb.max.z;
				p2 = p + mx*(aabb.max.x) + mz*aabb.max.z;
				p3 = p + mx*(aabb.max.x) + mz*(aabb.max.z+sizez);
				p4 = p + mx*(aabb.max.x-sizex) + mz*(aabb.max.z+sizez);
				break;
			default:
			case 0:
				p1 = p + mx*aabb.max.x + mz*aabb.min.z;
				p2 = p + mx*(aabb.max.x+sizex) + mz*aabb.min.z;
				p3 = p + mx*(aabb.max.x+sizex) + mz*(aabb.min.z+sizez);
				p4 = p + mx*aabb.max.x + mz*(aabb.min.z+sizez);
				break;
		}

		PutCityBit(rand, m, p1, p2, p3, p4);
	}
	Aabb buildAABB;
	for (std::vector<BuildingDef>::const_iterator iter=m_buildings.begin(), itEND=m_buildings.end(); iter != itEND; ++iter) {
		buildAABB.Update((*iter).pos - p);
	}
	m_realCentre = buildAABB.min + ((buildAABB.max - buildAABB.min)*0.5);
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
	if (m_detailLevel != Pi::detail.cities) {
		RemoveStaticGeomsFromCollisionSpace();
		AddStaticGeomsToCollisionSpace();
	}

	rot[0] = viewTransform * rot[0];
	for (int i=1; i<4; i++) {
		rot[i] = rot[0] * matrix4x4d::RotateYMatrix(M_PI*0.5*double(i));
	}
	for (int i=0; i<4; i++) {
		for (int e=0; e<16; e++) {
			rotf[i][e] = float(rot[i][e]);
		}
	}

	for (std::vector<BuildingDef>::const_iterator iter=m_enabledBuildings.begin(), itEND=m_enabledBuildings.end(); iter != itEND; ++iter)
	{
		const vector3d pos = viewTransform * (*iter).pos;
		const vector3f posf(pos);
		if (!frustum.TestPoint(pos, (*iter).clipRadius))
			continue;

		matrix4x4f _rot(rotf[(*iter).rotation]);
		_rot.SetTranslate(posf);

		(*iter).model->Render(_rot);
	}
}
