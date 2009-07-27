#include "libs.h"
#include "CityOnPlanet.h"
#include "Frame.h"
#include "SpaceStation.h"
#include "Planet.h"
#include "Pi.h"

#define START_SEG_SIZE 4000.0
#define MIN_SEG_SIZE 50.0

bool s_cityBuildingsInitted = false;
struct citybuilding_t {
	const char *modelname;
	int resolvedModelNum;
};

citybuilding_t city_buildings[] = {
	{ "building1" },
	{ "building2" },
	{ "building3" },
	{ "factory1" },
	{ "42" }, // a house
	{ "church" },
	{ 0 }
};

citybuilding_t wind_turbines[] = {
	{ "wind_turbine1" },
	{ "wind_turbine2" },
	{ "wind_turbine3" },
	{ 0 }
};

citybuilding_t starport_buildings[] = {
	{ "building1" },
	{ 0 }
};

#define MAX_BUILDING_LISTS 3
struct citybuildinglist_t {
	citybuilding_t *buildings;
	double minRadius, maxRadius;
	int numBuildings;
};

citybuildinglist_t s_buildingLists[MAX_BUILDING_LISTS] = {
	{ city_buildings, 400,1000 },
	{ wind_turbines, 100,250 },
	{ starport_buildings,300,400 },
};

#define CITYFLAVOURS 5
struct cityflavourdef_t {
	int buildingListIdx;
	vector3d center;
	double size;
} cityflavour[CITYFLAVOURS];


static Plane planes[6];
ObjParams cityobj_params;

void CityOnPlanet::PutCityBit(MTRand &rand, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4)
{
	double rad = (p1-p2).Length()*0.5;
	int modelNum;
	double modelRad;
	vector3d cent = (p1+p2+p3+p4)*0.25;

	cityflavourdef_t *flavour;
	citybuildinglist_t *buildings;

	// pick a building flavour (city, windfarm, etc)
	for (int flv=0; flv<CITYFLAVOURS; flv++) {
		flavour = &cityflavour[flv];
		buildings = &s_buildingLists[flavour->buildingListIdx];
       
		int tries;
		for (tries=20; tries--; ) {
			modelNum = buildings->buildings[rand.Int32(buildings->numBuildings)].resolvedModelNum;
			modelRad = sbreGetModelRadius(modelNum);
			if (modelRad < rad) break;
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

	if (rad > modelRad*2.0) {
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
		cent = cent * m_planet->GetTerrainHeight(cent);

		BuildingDef def = { modelNum, cent };
		m_buildings.push_back(def);
	}
}

static void lookupBuildingListModels(citybuildinglist_t *list)
{
	int i = 0;
	for (; list->buildings[i].modelname; i++) {
		list->buildings[i].resolvedModelNum = sbreLookupModelByName(list->buildings[i].modelname);
	}
	list->numBuildings = i;
}

CityOnPlanet::CityOnPlanet(const Planet *planet, const SpaceStation *station, Uint32 seed)
{
	m_buildings.clear();
	m_planet = planet;

	/* Resolve city model numbers since it is a bit expensive */
	if (!s_cityBuildingsInitted) {
		s_cityBuildingsInitted = true;
		for (int i=0; i<MAX_BUILDING_LISTS; i++) {
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
	cityflavour[0].buildingListIdx = 2;
	cityflavour[0].center = p;
	cityflavour[0].size = 500;

	for (int i=1; i<CITYFLAVOURS; i++) {
		cityflavour[i].buildingListIdx = rand.Int32(MAX_BUILDING_LISTS-1);
		citybuildinglist_t *blist = &s_buildingLists[cityflavour[i].buildingListIdx];
		int a, b;
		a = rand.Int32(-1000,1000);
		b = rand.Int32(-1000,1000);
		cityflavour[i].center = p + a*mx + b*mz;
		cityflavour[i].size = (double)rand.Int32(blist->minRadius, blist->maxRadius);
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

		sbreSetDepthRange(Pi::GetScrWidth()*0.5, 0.0f, 1.0f);
		vector3d center = (p1+p2+p3+p4)*0.25;
		PutCityBit(rand, m, p1, p2, p3, p4);
	}

}

void CityOnPlanet::Render(const SpaceStation *station, const Frame *camFrame)
{
	matrix4x4d m;
	station->GetRotMatrix(m);

	matrix4x4d frameTrans;
	Frame::GetFrameTransform(station->GetFrame(), camFrame, frameTrans);
	
	if ((frameTrans*station->GetPosition()).Length() > WORLDVIEW_ZFAR) {
		return;
	}
	
	m = frameTrans * m;
	
	GetFrustum(planes);
	
	memset(&cityobj_params, 0, sizeof(ObjParams));
	// this fucking rubbish needs to be moved into a function
	cityobj_params.pAnim[ASRC_SECFRAC] = (float)Pi::GetGameTime();
	cityobj_params.pAnim[ASRC_MINFRAC] = (float)(Pi::GetGameTime() / 60.0);
	cityobj_params.pAnim[ASRC_HOURFRAC] = (float)(Pi::GetGameTime() / 3600.0);
	cityobj_params.pAnim[ASRC_DAYFRAC] = (float)(Pi::GetGameTime() / (24*3600.0));

	for (std::vector<BuildingDef>::const_iterator i = m_buildings.begin();
			i != m_buildings.end(); ++i) {

		vector3d pos = frameTrans * (*i).pos;
		/* frustum cull */
		for (int j=0; j<6; j++) {
			if (planes[j].DistanceToPoint(pos)+sbreGetModelRadius((*i).modelnum) < 0) {
				continue;
			}
		}
		glPushMatrix();
		sbreRenderModel(&pos.x, &m[0], (*i).modelnum, &cityobj_params);
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}

