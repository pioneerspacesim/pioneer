#include "Planet.h"
#include "Pi.h"
#include "WorldView.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Graphics.h"
#include "graphics/VertexArray.h"

using namespace Graphics;
static const double ATMOSPHERE_RADIUS = 1.015;

struct ColRangeObj_t {
	float baseCol[4]; float modCol[4]; float modAll;

	void GenCol(float col[4], MTRand &rng) const {
		float ma = 1 + float(rng.Double(modAll*2)-modAll);
		for (int i=0; i<4; i++) col[i] = baseCol[i] + float(rng.Double(-modCol[i], modCol[i]));
		for (int i=0; i<3; i++) col[i] = Clamp(ma*col[i], 0.0f, 1.0f);
	}
};

ColRangeObj_t barrenBodyCol = { { .3f,.3f,.3f,1 },{0,0,0,0},.3f };
ColRangeObj_t barrenContCol = { { .2f,.2f,.2f,1 },{0,0,0,0},.3f };
ColRangeObj_t barrenEjectaCraterCol = { { .5f,.5f,.5f,1 },{0,0,0,0},.2f };
float darkblue[4] = { .05f, .05f, .2f, 1 };
float blue[4] = { .2f, .2f, 1, 1 };
float green[4] = { .2f, .8f, .2f, 1 };
float white[4] = { 1, 1, 1, 1 };

Planet::Planet(): TerrainBody()
{
}

Planet::Planet(SystemBody *sbody): TerrainBody(sbody)
{
	m_hasDoubleFrame = true;
}

/*
 * dist = distance from centre
 * returns pressure in earth atmospheres
 * function is slightly different from the isothermal earth-based approximation used in shaders,
 * but it isn't visually noticeable.
 */
void Planet::GetAtmosphericState(double dist, double *outPressure, double *outDensity) const
{
#if 0
	static int bool atmosphereTableShown = false;
	if (!atmosphereTableShown) {
		for (double h = -1000; h <= 50000; h = h+1000.0) {
			double p = 0.0, d = 0.0;
			GetAtmosphericState(h+this->GetSystemBody()->GetRadius(),&p,&d);
			printf("height(m): %f, pressure(kpa): %f, density: %f\n", h, p*101325.0/1000.0, d);
		}
		atmosphereTableShown = true;
	}
#endif

	double surfaceDensity;
	const double SPECIFIC_HEAT_AIR_CP=1000.5;// constant pressure specific heat, for the combination of gasses that make up air
	// XXX using earth's molar mass of air...
	const double GAS_MOLAR_MASS = 0.02897;
	const double GAS_CONSTANT = 8.3144621;
	const double PA_2_ATMOS = 1.0 / 101325.0;

	// surface gravity = -G*M/planet radius^2
	const double surfaceGravity_g = -G*this->GetSystemBody()->GetMass()/pow((this->GetSystemBody()->GetRadius()),2); // should be stored in sbody
	// lapse rate http://en.wikipedia.org/wiki/Adiabatic_lapse_rate#Dry_adiabatic_lapse_rate
	// the wet adiabatic rate can be used when cloud layers are incorporated
	// fairly accurate in the troposphere
	const double lapseRate_L = -surfaceGravity_g/SPECIFIC_HEAT_AIR_CP; // negative deg/m

	const double height_h = (dist-GetSystemBody()->GetRadius()); // height in m
	const double surfaceTemperature_T0 = this->GetSystemBody()->averageTemp; //K

	Color c;
	GetSystemBody()->GetAtmosphereFlavor(&c, &surfaceDensity);// kg / m^3
	// convert to moles/m^3
	surfaceDensity/=GAS_MOLAR_MASS;

	const double adiabaticLimit = surfaceTemperature_T0/lapseRate_L; //should be stored

	// This model has no atmosphere beyond the adiabetic limit
	if (height_h >= adiabaticLimit) {*outDensity = 0.0; *outPressure = 0.0; return;}

	//P = density*R*T=(n/V)*R*T
	const double surfaceP_p0 = PA_2_ATMOS*((surfaceDensity)*GAS_CONSTANT*surfaceTemperature_T0); // in atmospheres

	// height below zero should not occur
	if (height_h < 0.0) { *outPressure = surfaceP_p0; *outDensity = surfaceDensity; return; }

	//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
	*outPressure = pow(surfaceP_p0*(1-lapseRate_L*height_h/surfaceTemperature_T0),(-surfaceGravity_g*GAS_MOLAR_MASS/(GAS_CONSTANT*lapseRate_L)));// in ATM since p0 was in ATM
	//                                                                               ^^g used is abs(g)
	// temperature at height
	double temp = surfaceTemperature_T0+lapseRate_L*height_h;

	*outDensity = (*outPressure/(PA_2_ATMOS*GAS_CONSTANT*temp))*GAS_MOLAR_MASS;
}

struct GasGiantDef_t {
	int hoopMin, hoopMax; float hoopWobble;
	int blobMin, blobMax;
	float poleMin, poleMax; // size range in radians. zero for no poles.
	float ringProbability;
	ColRangeObj_t ringCol;
};

static const int NUM_GGDEFS = 5;
static GasGiantDef_t ggdefs[NUM_GGDEFS] = {
{
	/* jupiter */
	30, 40, 0.05f,
	20, 30,
	0, 0,
	0.5,
    { { .61f,.48f,.384f,.8f }, {0,0,0,.2}, 0.3f },
}, {
	/* saturnish */
	10, 25, 0.05f,
	8, 20, // blob range
	0.2f, 0.2f, // pole size
	0.5,
	{ { .61f,.48f,.384f,.85f }, {0,0,0,.1}, 0.3f },
}, {
	/* neptunish */
	3, 6, 0.25f,
	2, 6,
	0, 0,
	0.5,
    { { .71f,.68f,.684f,.8f }, {0,0,0,.1f}, 0.3f },
}, {
	/* uranus-like *wink* */
	2, 3, 0.1f,
	1, 2,
	0, 0,
	0.5,
	{ { .51f,.48f,.384f,.8f }, {0,0,0,.1f}, 0.3f },
}, {
	/* brown dwarf-like */
	0, 0, 0.05f,
	10, 20,
	0.2f, 0.2f,
	0.5,
    { { .81f,.48f,.384f,.8f }, {0,0,0,.1f}, 0.3f },
},
};

#define PLANET_AMBIENT	0.1f

static void DrawRing(double inner, double outer, const Color &color, Renderer *r, const Material &mat)
{
	float step = 0.1f / (Pi::detail.planets + 1);

	VertexArray vts(ATTRIB_POSITION | ATTRIB_DIFFUSE | ATTRIB_NORMAL);
	const vector3f normal(0.f, 1.f, 0.f);
	for (float ang=0; ang<2*M_PI; ang+=step) {
		vts.Add(vector3f(float(inner)*sin(ang), 0.f, float(inner)*cos(ang)), color, normal);
		vts.Add(vector3f(float(outer)*sin(ang), 0.f, float(outer)*cos(ang)), color, normal);
	}
	vts.Add(vector3f(0.f, 0.f, float(inner)), color, normal);
	vts.Add(vector3f(0.f, 0.f, float(outer)), color, normal);

	r->DrawTriangles(&vts, &mat, TRIANGLE_STRIP);
}

void Planet::DrawGasGiantRings(Renderer *renderer, const Camera *camera)
{
	renderer->SetBlendMode(BLEND_ALPHA_ONE);
	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT );
	renderer->SetDepthTest(true);
	glEnable(GL_NORMALIZE);

	Material mat;
	mat.unlit = true;
	mat.twoSided = true;
	// XXX should get number of lights through camera when object viewer draw doesn't pass a null pointer
	mat.shader = Graphics::planetRingsShader[Graphics::State::GetNumLights()-1];

//	MTRand rng((int)Pi::game->GetTime());
	MTRand rng(GetSystemBody()->seed+965467);

	double noiseOffset = 256.0*rng.Double();
	float baseCol[4];

	// just use a random gas giant flavour for the moment
	GasGiantDef_t &ggdef = ggdefs[rng.Int32(COUNTOF(ggdefs))];
	ggdef.ringCol.GenCol(baseCol, rng);

	const double maxRingWidth = 0.1 / double(2*(Pi::detail.planets + 1));

	if (rng.Double(1.0) < ggdef.ringProbability) {
		float rpos = float(rng.Double(1.15,1.5));
		float end = rpos + float(rng.Double(0.1, 1.0));
		end = std::min(end, 2.5f);
		while (rpos < end) {
			float size = float(rng.Double(maxRingWidth));
			float n =
				0.5 + 0.5*(
					noise(10.0*rpos, noiseOffset, 0.0) +
					0.5*noise(20.0*rpos, noiseOffset, 0.0) +
					0.25*noise(40.0*rpos, noiseOffset, 0.0));
			Color col(baseCol[0] * n, baseCol[1] * n, baseCol[2] * n, baseCol[3] * n);
			DrawRing(rpos, rpos+size, col, renderer, mat);
			rpos += size;
		}
	}

	glPopAttrib();
	renderer->SetBlendMode(BLEND_SOLID);
}

void Planet::DrawAtmosphere(Renderer *renderer, const vector3d &camPos)
{
	//this is the non-shadered atmosphere rendering
	Color col;
	double density;
	GetSystemBody()->GetAtmosphereFlavor(&col, &density);

	const double rad1 = 0.999;
	const double rad2 = 1.05;

	glPushMatrix();

	//XXX pass the transform
	matrix4x4d curTrans;
	glGetDoublev(GL_MODELVIEW_MATRIX, &curTrans[0]);

	// face the camera dammit
	vector3d zaxis = (-camPos).Normalized();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
	const matrix4x4d trans = curTrans * rot;

	matrix4x4d invViewRot = trans;
	invViewRot.ClearToRotOnly();
	invViewRot = invViewRot.InverseOf();

	//XXX this is always 1
	const int numLights = Pi::worldView->GetNumLights();
	assert(numLights < 4);
	vector3d lightDir[4];
	float lightCol[4][4];
	// only
	for (int i=0; i<numLights; i++) {
		float temp[4];
		glGetLightfv(GL_LIGHT0 + i, GL_DIFFUSE, lightCol[i]);
		glGetLightfv(GL_LIGHT0 + i, GL_POSITION, temp);
		lightDir[i] = (invViewRot * vector3d(temp[0], temp[1], temp[2])).Normalized();
	}

	const double angStep = M_PI/32;
	// find angle player -> centre -> tangent point
	// tangent is from player to surface of sphere
	float tanAng = float(acos(rad1 / camPos.Length()));

	// then we can put the fucking atmosphere on the horizon
	vector3d r1(0.0, 0.0, rad1);
	vector3d r2(0.0, 0.0, rad2);
	rot = matrix4x4d::RotateYMatrix(tanAng);
	r1 = rot * r1;
	r2 = rot * r2;

	rot = matrix4x4d::RotateZMatrix(angStep);

	VertexArray vts(ATTRIB_POSITION | ATTRIB_DIFFUSE | ATTRIB_NORMAL);
	for (float ang=0; ang<2*M_PI; ang+=float(angStep)) {
		const vector3d norm = r1.Normalized();
		const vector3f n = vector3f(norm.x, norm.y, norm.z);
		float _col[4] = { 0,0,0,0 };
		for (int lnum=0; lnum<numLights; lnum++) {
			const float dot = norm.x*lightDir[lnum].x + norm.y*lightDir[lnum].y + norm.z*lightDir[lnum].z;
			_col[0] += dot*lightCol[lnum][0];
			_col[1] += dot*lightCol[lnum][1];
			_col[2] += dot*lightCol[lnum][2];
		}
		for (int i=0; i<3; i++) _col[i] = _col[i] * col[i];
		_col[3] = col[3];
		vts.Add(vector3f(r1.x, r1.y, r1.z), Color(_col[0], _col[1], _col[2], _col[3]), n);
		vts.Add(vector3f(r2.x, r2.y, r2.z), Color(0.f), n);
		r1 = rot * r1;
		r2 = rot * r2;
	}

	Material mat;
	mat.unlit = true;
	mat.twoSided = true;
	mat.vertexColors = true;

	renderer->SetTransform(trans);
	renderer->SetBlendMode(BLEND_ALPHA_ONE);
	renderer->DrawTriangles(&vts, &mat, TRIANGLE_STRIP);
	renderer->SetBlendMode(BLEND_SOLID);

	glPopMatrix();
}

void Planet::SubRender(Renderer *r, const Camera *camera, const vector3d &camPos)
{
	if (GetSystemBody()->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT) DrawGasGiantRings(r, camera);

	if (!AreShadersEnabled()) DrawAtmosphere(r, camPos);
}
