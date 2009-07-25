#include "libs.h"
#include "CityOnPlanet.h"
#include "Frame.h"
#include "SpaceStation.h"
#include "Planet.h"
#include "Pi.h"

#define START_SEG_SIZE 2000.0
#define DIVIDE_SEG_SIZE 100.0
#define MAX_CITY_BUILDING 9

bool s_cityBuildingsInitted = false;
struct citybuilding_t {
	const char *modelname;
	int resolvedModelNum;
} city_buildings[MAX_CITY_BUILDING] = {
	{ "building1" },
	{ "building2" },
	{ "building3" },
	{ "factory1" },
	{ "42" }, // a house
	{ "church" },
	{ "wind_turbine1" },
	{ "wind_turbine2" },
	{ "wind_turbine3" },
};

static Plane planes[6];
ObjParams cityobj_params;

static void drawModel(const matrix4x4d &rot, vector3d pos, int modelNum)
{
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

//	glPushAttrib(GL_ALL_ATTRIB_BITS);
	sbreRenderModel(&pos.x, &rot[0], modelNum, &cityobj_params);
//	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static void putCityBit(const vector3d &cityCenter, const Planet *planet, MTRand &rand,
		const matrix4x4d &frameTrans, const matrix4x4d &rot, vector3d p1, vector3d p2, vector3d p3, vector3d p4)
{
	double rad = (p1-p2).Length()*0.5;
	int modelNum;
	double modelRad;
       
	int tries;
	for (tries=10; tries--; ) {
       		modelNum = city_buildings[rand.Int32(MAX_CITY_BUILDING)].resolvedModelNum;
		modelRad = sbreGetModelRadius(modelNum);
		if (modelRad < rad) break;
		if (tries == 0) return;
	}

	if ((rad > DIVIDE_SEG_SIZE) || (rad > modelRad*2.0)) {
		vector3d a = (p1+p2)*0.5;
		vector3d b = (p2+p3)*0.5;
		vector3d c = (p3+p4)*0.5;
		vector3d d = (p4+p1)*0.5;
		vector3d e = (p1+p2+p3+p4)*0.25;
		putCityBit(cityCenter, planet, rand, frameTrans, rot, p1, a, e, d);
		putCityBit(cityCenter, planet, rand, frameTrans, rot, a, p2, b, e);
		putCityBit(cityCenter, planet, rand, frameTrans, rot, e, b, p3, c);
		putCityBit(cityCenter, planet, rand, frameTrans, rot, d, e, c, p4);
	} else {
		vector3d cent = (p1+p2+p3+p4)*0.25;

		// outskirts less densely populated
		if ((cityCenter-cent).Length()*(1.0/START_SEG_SIZE) > rand.Double()) return;
		
		cent = cent.Normalized();
		cent = cent * planet->GetTerrainHeight(cent);
		cent = frameTrans * cent;

		/* frustum cull */
		for (int i=0; i<6; i++) {
			if (planes[i].DistanceToPoint(cent)+sbreGetModelRadius(modelNum) < 0) {
				return;
			}
		}

		drawModel(rot, cent, modelNum);
	/*	
		p1 = frameTrans*p1;
		p2 = frameTrans*p2;
		p3 = frameTrans*p3;
		p4 = frameTrans*p4;
		
		glDisable(GL_LIGHTING);
		glColor3f(0.5,.5,.5);
		glBegin(GL_LINE_LOOP);
			glVertex3dv(&p1.x);
			glVertex3dv(&p2.x);
			glVertex3dv(&p3.x);
			glVertex3dv(&p4.x);
		glEnd();
		glEnable(GL_LIGHTING);
	*/
	}
}

namespace CityOnPlanet {

void Render(const Planet *planet, const SpaceStation *station, const Frame *camFrame, Uint32 seed)
{
	/* Resolve city model numbers since it is a bit expensive */
	if (!s_cityBuildingsInitted) {
		s_cityBuildingsInitted = true;
		for (int i=0; i<MAX_CITY_BUILDING; i++) {
			city_buildings[i].resolvedModelNum = sbreLookupModelByName(city_buildings[i].modelname);
		}
	}

	Aabb aabb;
	station->GetAabb(aabb);
	
	matrix4x4d m;
	station->GetRotMatrix(m);

	vector3d mx = m*vector3d(1,0,0);
	vector3d mz = m*vector3d(0,0,1);
		
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
	MTRand rand;
	rand.seed(seed);

	vector3d p = station->GetPosition();
	vector3d p1, p2, p3, p4;
	const double sizex = START_SEG_SIZE + rand.Int32((int)START_SEG_SIZE);
	const double sizez = START_SEG_SIZE + rand.Int32((int)START_SEG_SIZE);
	
	/* put city on random side of spaceport */
	switch(rand.Int32(4)) {
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

	vector3d centre = (p1+p2+p3+p4)*0.25;
	putCityBit(centre, planet, rand, frameTrans, m, p1, p2, p3, p4);

}

}
