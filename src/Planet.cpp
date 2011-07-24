#include "libs.h"
#include "Planet.h"
#include "Frame.h"
#include "Pi.h"
#include "WorldView.h"
#include "Serializer.h"
#include "StarSystem.h"
#include "GeoSphere.h"
#include "Render.h"
#include "perlin.h"

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

Planet::Planet(): Body()
{
	pos = vector3d(0,0,0);
	this->sbody = 0;
	this->m_geosphere = 0;
}

Planet::Planet(SBody *sbody_): Body()
{
	pos = vector3d(0,0,0);
	this->sbody = sbody_;
	this->m_geosphere = 0;
	Init();
	m_hasDoubleFrame = true;
}

void Planet::Init()
{
	m_mass = sbody->GetMass();
	if (!m_geosphere) {
		m_geosphere = new GeoSphere(sbody);
	}
}

bool Planet::IsSuperType(SBody::BodySuperType t) const
{
	if (!sbody) return false;
	else return sbody->GetSuperType() == t;
}
	
void Planet::Save(Serializer::Writer &wr)
{
	Body::Save(wr);
	wr.Vector3d(pos);
	wr.Int32(Serializer::LookupSystemBody(sbody));
}

void Planet::Load(Serializer::Reader &rd)
{
	Body::Load(rd);
	pos = rd.Vector3d();
	sbody = Serializer::LookupSystemBody(rd.Int32());
	Init();
}

Planet::~Planet()
{
	if (m_geosphere) delete m_geosphere;
}

double Planet::GetBoundingRadius() const
{
	// needs to include all terrain so culling works {in the future},
	// and size of rotating frame is correct
	return sbody->GetRadius() * (1.1+m_geosphere->GetMaxFeatureHeight());
}

vector3d Planet::GetPosition() const
{
	return pos;
}

void Planet::SetPosition(vector3d p)
{
	pos = p;
}

void Planet::SetRadius(double radius)
{
	assert(0);
}

/*
 * dist = distance from centre
 * returns pressure in earth atmospheres
 */
void Planet::GetAtmosphericState(double dist, double *outPressure, double *outDensity)
{
	Color c;
	double surfaceDensity;
	double atmosDist = dist/(sbody->GetRadius()*ATMOSPHERE_RADIUS);
	
	m_geosphere->GetAtmosphereFlavor(&c, &surfaceDensity);
	// kg / m^3
	// exp term should be the same as in AtmosLengthDensityProduct GLSL function
	*outDensity = 1.15*surfaceDensity * exp(-500.0 * (atmosDist - (2.0 - ATMOSPHERE_RADIUS)));
	// XXX using earth's molar mass of air...
	const double GAS_MOLAR_MASS = 28.97;
	const double GAS_CONSTANT = 8.314;
	const double KPA_2_ATMOS = 1.0 / 101.325;
	// atmospheres
	*outPressure = KPA_2_ATMOS*(*outDensity/GAS_MOLAR_MASS)*GAS_CONSTANT*double(sbody->averageTemp);
}

double Planet::GetTerrainHeight(const vector3d pos_) const
{
	double radius = sbody->GetRadius();
	if (m_geosphere) {
		return radius * (1.0 + m_geosphere->GetHeight(pos_));
	} else {
		assert(0);
		return radius;
	}
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

static void SetMaterialColor(const float col[4])
{
	float mambient[4];
	mambient[0] = col[0]*PLANET_AMBIENT;
	mambient[1] = col[1]*PLANET_AMBIENT;
	mambient[2] = col[2]*PLANET_AMBIENT;
	mambient[3] = col[3];
	glMaterialfv (GL_FRONT, GL_AMBIENT, mambient);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, col);
}

static void DrawRing(double inner, double outer, const float color[4])
{
	glColor4fv(color);

	float step = 0.1f / (Pi::detail.planets + 1);

	glBegin(GL_TRIANGLE_STRIP);
	glNormal3f(0,1,0);
	for (float ang=0; ang<2*M_PI; ang+=step) {
		glVertex3f(float(inner)*sin(ang), 0, float(inner)*cos(ang));
		glVertex3f(float(outer)*sin(ang), 0, float(outer)*cos(ang));
	}
	glVertex3f(0, 0, float(inner));
	glVertex3f(0, 0, float(outer));
	glEnd();
}

void Planet::DrawGasGiantRings()
{
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
		GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glDisable(GL_CULL_FACE);

//	MTRand rng((int)Pi::GetGameTime());
	MTRand rng(sbody->seed+965467);
	float col[4];

	double noiseOffset = 256.0*rng.Double();
	float baseCol[4];
	
	// just use a random gas giant flavour for the moment
	GasGiantDef_t &ggdef = ggdefs[rng.Int32(0,4)];
	ggdef.ringCol.GenCol(baseCol, rng);
	
	const double maxRingWidth = 0.1 / double(2*(Pi::detail.planets + 1));

	Render::State::UseProgram(Render::planetRingsShader[Pi::worldView->GetNumLights()-1]);
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
			DrawRing(rpos, rpos+size, col);
			rpos += size;
		}
	}
	Render::State::UseProgram(0);
	
	glEnable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glDisable(GL_BLEND);
	glDisable(GL_NORMALIZE);
	glPopAttrib();
}

static void _DrawAtmosphere(double rad1, double rad2, vector3d &pos, const float col[4])
{
	glPushMatrix();
	// face the camera dammit
	vector3d zaxis = (-pos).Normalized();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
	glMultMatrixd(&rot[0]);

	matrix4x4f invViewRot;
	glGetFloatv(GL_MODELVIEW_MATRIX, &invViewRot[0]);
	invViewRot.ClearToRotOnly();
	invViewRot = invViewRot.InverseOf();

	const int numLights = Pi::worldView->GetNumLights();
	assert(numLights < 4);
	vector3f lightDir[4];
	float lightCol[4][4];
	// only 
	for (int i=0; i<numLights; i++) {
		float temp[4];
		glGetLightfv(GL_LIGHT0 + i, GL_DIFFUSE, lightCol[i]);
		glGetLightfv(GL_LIGHT0 + i, GL_POSITION, temp);
		lightDir[i] = (invViewRot * vector3f(temp[0], temp[1], temp[2])).Normalized();
	}

	const double angStep = M_PI/32;
	// find angle player -> centre -> tangent point
	// tangent is from player to surface of sphere
	float tanAng = float(acos(rad1 / pos.Length()));

	// then we can put the fucking atmosphere on the horizon
	vector3d r1(0.0, 0.0, rad1);
	vector3d r2(0.0, 0.0, rad2);
	rot = matrix4x4d::RotateYMatrix(tanAng);
	r1 = rot * r1;
	r2 = rot * r2;

	rot = matrix4x4d::RotateZMatrix(angStep);

	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLE_STRIP);
	for (float ang=0; ang<2*M_PI; ang+=float(angStep)) {
		vector3d norm = r1.Normalized();
		glNormal3dv(&norm.x);
		float _col[4] = { 0,0,0,0 };
		for (int lnum=0; lnum<numLights; lnum++) {
			const float dot = norm.x*lightDir[lnum].x + norm.y*lightDir[lnum].y + norm.z*lightDir[lnum].z;
			_col[0] += dot*lightCol[lnum][0];
			_col[1] += dot*lightCol[lnum][1];
			_col[2] += dot*lightCol[lnum][2];
		}
		for (int i=0; i<3; i++) _col[i] = _col[i] * col[i];
		_col[3] = col[3];
		glColor4fv(_col);
		glVertex3dv(&r1.x);
		glColor4f(0,0,0,0);
		glVertex3dv(&r2.x);
		r1 = rot * r1;
		r2 = rot * r2;
	}
	
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void Planet::DrawAtmosphere(vector3d &apos)
{
	Color c;
	double density;
	m_geosphere->GetAtmosphereFlavor(&c, &density);
	
	_DrawAtmosphere(0.999, 1.05, apos, c);
}

void Planet::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	matrix4x4d ftran = viewTransform;
	vector3d fpos = viewCoords;
	double rad = sbody->GetRadius();

	float znear, zfar;
	Pi::worldView->GetNearFarClipPlane(&znear, &zfar);

	double apparent_size = rad / fpos.Length();
	double len = fpos.Length();
	double origLen = len;
	int shrink = 0;
	double scale = 1.0f;

	double dist_to_horizon;
	for (;;) {
		if (len < rad) break;		// player inside radius case
		dist_to_horizon = sqrt(len*len - rad*rad);

		if (dist_to_horizon < zfar*0.5) break;

		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
		scale *= 4.0f;
		shrink++;
	}
	//if (GetLabel() == "Earth") printf("Horizon %fkm, shrink %d\n", dist_to_horizon*0.001, shrink);

	glPushMatrix();		// initial matrix is actually identity after a long chain of wtf
//	glTranslatef(float(fpos.x), float(fpos.y), float(fpos.z));
	glColor3f(1,1,1);

	{
		vector3d campos = fpos;
		ftran.ClearToRotOnly();
		campos = ftran.InverseOf() * campos;
		glMultMatrixd(&ftran[0]);
		glEnable(GL_NORMALIZE);
		glScaled(rad, rad, rad);			// rad = real_rad / scale
		campos = campos * (1.0/rad);		// position of camera relative to planet "model"

		// geosphere->Render translates by campos internally to avoid jitter
//		glTranslated(campos.x, campos.y, campos.z);
		m_geosphere->Render(-campos, sbody->GetRadius(), scale);
		glTranslated(campos.x, campos.y, campos.z);

		if (sbody->GetSuperType() == SBody::SUPERTYPE_GAS_GIANT) DrawGasGiantRings();
		
		if (!Render::AreShadersEnabled()) DrawAtmosphere(campos);
		
		glDisable(GL_NORMALIZE);
		
		// if not using shader then z-buffer precision is hopeless and
		// we can't place objects on the terrain without awful z artifacts
		if (shrink || !Render::AreShadersEnabled()) {
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}
	glPopMatrix();
}

void Planet::SetFrame(Frame *f)
{
	if (GetFrame()) {
		GetFrame()->SetPlanetGeom(0, 0);
	}
	Body::SetFrame(f);
	if (f) {
		GetFrame()->SetPlanetGeom(0, 0);
	}
}

