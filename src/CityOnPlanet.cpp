#include "libs.h"
#include "CityOnPlanet.h"
#include "Frame.h"
#include "SpaceStation.h"
#include "Planet.h"
#include "Pi.h"
#include "Game.h"
#include "collider/Geom.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"

#define START_SEG_SIZE CITY_ON_PLANET_RADIUS
#define MIN_SEG_SIZE 50.0

bool s_cityBuildingsInitted = false;
struct citybuilding_t {
	const char *modelname;
	double xzradius;
	LmrModel *resolvedModel;
	const LmrCollMesh *collMesh;
};

struct citybuildinglist_t {
	const char *modelTagName;
	double minRadius, maxRadius;
	int numBuildings;
	citybuilding_t *buildings;
};

static citybuildinglist_t s_buildingLists[] = {
	{ "city_building", 800, 2000, 0, 0 },
	//{ "city_power", 100, 250, 0, 0 },
	//{ "city_starport_building", 300, 400, 0, 0 },
};

#define CITYFLAVOURS 5
struct cityflavourdef_t {
	int buildingListIdx;
	vector3d center;
	double size;
} cityflavour[CITYFLAVOURS];


LmrObjParams cityobj_params;

void CityOnPlanet::PutCityBit(MTRand &rand, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4)
{
	double rad = (p1-p2).Length()*0.5;
	LmrModel *model(0);
	double modelRadXZ(0);
	const LmrCollMesh *cmesh(0);
	vector3d cent = (p1+p2+p3+p4)*0.25;

	cityflavourdef_t *flavour(0);
	citybuildinglist_t *buildings(0);

	// pick a building flavour (city, windfarm, etc)
	for (int flv=0; flv<CITYFLAVOURS; flv++) {
		flavour = &cityflavour[flv];
		buildings = &s_buildingLists[flavour->buildingListIdx];

		int tries;
		for (tries=20; tries--; ) {
			const citybuilding_t &bt = buildings->buildings[rand.Int32(buildings->numBuildings)];
			model = bt.resolvedModel;
			modelRadXZ = bt.xzradius;
			cmesh = bt.collMesh;
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

		assert(cmesh);
		Geom *geom = new Geom(cmesh->geomTree);
		int rotTimes90 = rand.Int32(4);
		matrix4x4d grot = rot * matrix4x4d::RotateYMatrix(M_PI*0.5*double(rotTimes90));
		geom->MoveTo(grot, cent);
		geom->SetUserData(this);
//		f->AddStaticGeom(geom);

		BuildingDef def = { model, cmesh->GetBoundingRadius(), rotTimes90, cent, geom, false };
		m_buildings.push_back(def);
	}
}

void CityOnPlanet::AddStaticGeomsToCollisionSpace()
{
	int skipMask;
	switch (Pi::detail.cities) {
		case 0: skipMask = 0xf; break;
		case 1: skipMask = 0x7; break;
		case 2: skipMask = 0x3; break;
		case 3: skipMask = 0x1; break;
		default:
			skipMask = 0; break;
	}
	for (unsigned int i=0; i<m_buildings.size(); i++) {
		if (i & skipMask) {
			m_buildings[i].isEnabled = false;
		} else {
			m_frame->AddStaticGeom(m_buildings[i].geom);
			m_buildings[i].isEnabled = true;
		}
	}
	m_detailLevel = Pi::detail.cities;
}

void CityOnPlanet::RemoveStaticGeomsFromCollisionSpace()
{
	for (unsigned int i=0; i<m_buildings.size(); i++) {
		m_frame->RemoveStaticGeom(m_buildings[i].geom);
		m_buildings[i].isEnabled = false;
	}
}

static void lookupBuildingListModels(citybuildinglist_t *list)
{
	//const char *modelTagName;
	std::vector<LmrModel*> models;
	LmrGetModelsWithTag(list->modelTagName, models);
	//printf("Got %d buildings of tag %s\n", models.size(), list->modelTagName);
	list->buildings = new citybuilding_t[models.size()];
	list->numBuildings = models.size();

	int i = 0;
	for (std::vector<LmrModel*>::iterator m = models.begin(); m != models.end(); ++m, i++) {
		list->buildings[i].resolvedModel = *m;
		const LmrCollMesh *collMesh = new LmrCollMesh(*m, &cityobj_params);
		list->buildings[i].collMesh = collMesh;
		double maxx = std::max(fabs(collMesh->GetAabb().max.x), fabs(collMesh->GetAabb().min.x));
		double maxy = std::max(fabs(collMesh->GetAabb().max.z), fabs(collMesh->GetAabb().min.z));
		list->buildings[i].xzradius = sqrt(maxx*maxx + maxy*maxy);
		//printf("%s: %f\n", list->buildings[i].modelname, list->buildings[i].xzradius);
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
		for (int build=0; build<s_buildingLists[list].numBuildings; build++) {
			delete s_buildingLists[list].buildings[build].collMesh;
		}
		delete[] s_buildingLists[list].buildings;
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

CityOnPlanet::CityOnPlanet(Planet *planet, SpaceStation *station, Uint32 seed)
{
	m_buildings.clear();
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

	Aabb aabb;
	station->GetAabb(aabb);

	matrix4x4d m;
	station->GetRotMatrix(m);

	vector3d mx = m*vector3d(1,0,0);
	vector3d mz = m*vector3d(0,0,1);

	MTRand rand;
	rand.seed(seed);

	vector3d p = station->GetPosition();

	vector3d p1, p2, p3, p4;
	double sizex = START_SEG_SIZE;// + rand.Int32((int)START_SEG_SIZE);
	double sizez = START_SEG_SIZE;// + rand.Int32((int)START_SEG_SIZE);

	// always have random shipyard buildings around the space station
	cityflavour[0].buildingListIdx = 0;//2;
	cityflavour[0].center = p;
	cityflavour[0].size = 500;

	for (int i=1; i<CITYFLAVOURS; i++) {
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
	AddStaticGeomsToCollisionSpace();
}

//Note: models get some ambient colour added when dark as the camera moves closer
void CityOnPlanet::Render(Graphics::Renderer *r, const Camera *camera, const SpaceStation *station, const vector3d &viewCoords, const matrix4x4d &viewTransform, double illumination, double minIllumination)
{
	matrix4x4d rot[4];
	station->GetRotMatrix(rot[0]);

	// change detail level if necessary
	if (m_detailLevel != Pi::detail.cities) {
		RemoveStaticGeomsFromCollisionSpace();
		AddStaticGeomsToCollisionSpace();
	}

	rot[0] = viewTransform * rot[0];
	for (int i=1; i<4; i++) {
		rot[i] = rot[0] * matrix4x4d::RotateYMatrix(M_PI*0.5*double(i));
	}

	Graphics::Frustum frustum = Graphics::Frustum::FromGLState();
	//modelview seems to be always identity

	memset(&cityobj_params, 0, sizeof(LmrObjParams));
	cityobj_params.time = Pi::game->GetTime();
	
	for (std::vector<BuildingDef>::const_iterator i = m_buildings.begin();
			i != m_buildings.end(); ++i) {

		if (!(*i).isEnabled) continue;
		
		vector3d pos = viewTransform * (*i).pos;
		if (!frustum.TestPoint(pos, (*i).clipRadius))
			continue;

		Color oldSceneAmbientColor;
		if (illumination <= minIllumination)
			oldSceneAmbientColor = Graphics::State::GetGlobalSceneAmbientColor();

		// fade conditions for models
		double fadeInEnd, fadeInLength;
		if (Graphics::AreShadersEnabled()) {
			fadeInEnd = 10.0;
			fadeInLength = 500.0;
		}
		else {
			fadeInEnd = 2000.0;
			fadeInLength = 6000.0;
		}

		FadeInModelIfDark(r, (*i).clipRadius, pos.Length(), fadeInEnd, fadeInLength, illumination, minIllumination);

		matrix4x4f _rot;
		for (int e=0; e<16; e++) _rot[e] = float(rot[(*i).rotation][e]);
		_rot[12] = float(pos.x);
		_rot[13] = float(pos.y);
		_rot[14] = float(pos.z);
		(*i).model->Render(_rot, &cityobj_params);

		// restore old ambient colour
		if (illumination <= minIllumination) 
			r->SetAmbientColor(oldSceneAmbientColor);
	}
}

