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

Planet::Planet(SBody *sbody): TerrainBody(sbody)
{
	m_hasDoubleFrame = true;
}

/*
 * dist = distance from centre
 * returns pressure in earth atmospheres
 */
void Planet::GetAtmosphericState(double dist, double *outPressure, double *outDensity)
{
	Color c;
	double surfaceDensity;
	double atmosDist = dist/(GetSBody()->GetRadius()*ATMOSPHERE_RADIUS);
	
	GetSBody()->GetAtmosphereFlavor(&c, &surfaceDensity);
	// kg / m^3
	// exp term should be the same as in AtmosLengthDensityProduct GLSL function
	*outDensity = 1.15*surfaceDensity * exp(-500.0 * (atmosDist - (2.0 - ATMOSPHERE_RADIUS)));
	// XXX using earth's molar mass of air...
	const double GAS_MOLAR_MASS = 28.97;
	const double GAS_CONSTANT = 8.314;
	const double KPA_2_ATMOS = 1.0 / 101.325;
	// atmospheres
	*outPressure = KPA_2_ATMOS*(*outDensity/GAS_MOLAR_MASS)*GAS_CONSTANT*double(GetSBody()->averageTemp);
}

struct GasGiantDef_t {
	int hoopMin, hoopMax; float hoopWobble;
	int blobMin, blobMax;
	float poleMin, poleMax; // size range in radians. zero for no poles.
	float ringProbability;
	ColRangeObj_t ringCol;
	ColRangeObj_t bodyCol;
	ColRangeObj_t hoopCol;
	ColRangeObj_t blobCol;
	ColRangeObj_t poleCol;
};

static GasGiantDef_t ggdefs[] = {
{
	/* jupiter */
	30, 40, 0.05f,
	20, 30,
	0, 0,
	0.5,
	{ { .61f,.48f,.384f,.4f }, {0,0,0,.2}, 0.3f },
	{ { .99f,.76f,.62f,1 }, { 0,.1f,.1f,0 }, 0.3f },
	{ { .99f,.76f,.62f,.5f }, { 0,.1f,.1f,0 }, 0.3f },
	{ { .99f,.76f,.62f,1 }, { 0,.1f,.1f,0 }, 0.7f },
    { { 0.0f,0.0f,0.0f,0 }, { 0,0.0f,0.0f,0}, 0.0f}
}, {
	/* saturnish */
	10, 25, 0.05f,
	8, 20, // blob range
	0.2f, 0.2f, // pole size
	0.5,
	{ { .61f,.48f,.384f,.75f }, {0,0,0,.2}, 0.3f },
	{ { .87f, .68f, .39f, 1 }, { .2,0,0,.3 }, 0.1f },
	{ { .87f, .68f, .39f, 1 }, { 0,0,.2,.1 }, 0.1f },
	{ { .87f, .68f, .39f, 1 }, { .1,0,0,.1 }, 0.1f },
	{ { .77f, .58f, .29f, 1 }, { .1,.1,0,.2 }, 0.2f },
}, {
	/* neptunish */
	3, 6, 0.25f,
	2, 6,
	0, 0,
	0.5,
	{ { .71f,.68f,.684f,.4f }, {0,0,0,.2f}, 0.3f },
	{ { .31f,.44f,.73f,1 }, {0,0,0,0}, .05f}, // body col
	{ { .31f,.74f,.73f,0.5f }, {0,0,0,0}, .1f},// hoop col
	{ { .21f,.34f,.54f,1 }, {0,0,0,0}, .05f},// blob col
    { { 0.0f,0.0f,0.0f,0 }, { 0,0.0f,0.0f,0}, 0.0f}
}, {
	/* uranus-like *wink* */
	2, 3, 0.1f,
	1, 2,
	0, 0,
	0.5,
	{ { .51f,.48f,.384f,.4f }, {0,0,0,.3f}, 0.3f },
	{ { .60f,.85f,.86f,1 }, {.1f,.1f,.1f,0}, 0 },
	{ { .60f,.85f,.86f,1 }, {.1f,.1f,.1f,0}, 0 },
	{ { .60f,.85f,.86f,1 }, {.1f,.1f,.1f,0}, 0 },
	{ { .60f,.85f,.86f,1 }, {.1f,.1f,.1f,0}, 0 }
}, {
	/* brown dwarf-like */
	0, 0, 0.05f,
	10, 20,
	0.2f, 0.2f,
	0.5,
	{ { .81f,.48f,.384f,.5f }, {0,0,0,.3f}, 0.3f },
	{ { .4f,.1f,0,1 }, {0,0,0,0}, 0.1f },
	{ { .4f,.1f,0,1 }, {0,0,0,0}, 0.1f },
	{ { .4f,.1f,0,1 }, {0,0,0,0}, 0.1f },
    { { 0.0f,0.0f,0.0f,0 }, { 0,0.0f,0.0f,0}, 0.0f}
},
};

#define PLANET_AMBIENT	0.1f

static void DrawRing(double inner, double outer, const float color[4], Renderer *r, const Material &mat)
{
	float step = 0.1f / (Pi::detail.planets + 1);

	VertexArray vts(ATTRIB_POSITION | ATTRIB_DIFFUSE | ATTRIB_NORMAL);
	const vector3f normal(0.f, 1.f, 0.f);
	const Color c(color[0], color[1], color[2], color[3]);
	for (float ang=0; ang<2*M_PI; ang+=step) {
		vts.Add(vector3f(float(inner)*sin(ang), 0.f, float(inner)*cos(ang)), c, normal);
		vts.Add(vector3f(float(outer)*sin(ang), 0.f, float(outer)*cos(ang)), c, normal);
	}
	vts.Add(vector3f(0.f, 0.f, float(inner)), c, normal);
	vts.Add(vector3f(0.f, 0.f, float(outer)), c, normal);

	r->DrawTriangles(&vts, &mat, TRIANGLE_STRIP);
}

void Planet::DrawGasGiantRings(Renderer *renderer)
{
	renderer->SetBlendMode(BLEND_ALPHA_ONE);
	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT );
	renderer->SetDepthTest(true);
	glEnable(GL_NORMALIZE);

	Material mat;
	mat.unlit = true;
	mat.twoSided = true;
	// XXX worldview numlights always 1!
	mat.shader = Graphics::planetRingsShader[Pi::worldView->GetNumLights()-1];

//	MTRand rng((int)Pi::game->GetTime());
	MTRand rng(GetSBody()->seed+965467);
	float col[4];

	double noiseOffset = 256.0*rng.Double();
	float baseCol[4];
	
	// just use a random gas giant flavour for the moment
	GasGiantDef_t &ggdef = ggdefs[rng.Int32(0,4)];
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
			col[0] = baseCol[0] * n;
			col[1] = baseCol[1] * n;
			col[2] = baseCol[2] * n;
			col[3] = baseCol[3] * n;
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
	GetSBody()->GetAtmosphereFlavor(&col, &density);
	
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
		vts.Add(vector3f(r1.x, r1.y, r1.z), Color(_col), n);
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

void Planet::SubRender(Renderer *r, const vector3d &camPos)
{
	if (GetSBody()->GetSuperType() == SBody::SUPERTYPE_GAS_GIANT) DrawGasGiantRings(r);
	
	if (!AreShadersEnabled()) DrawAtmosphere(r, camPos);
}
