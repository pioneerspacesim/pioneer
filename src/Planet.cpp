// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Planet.h"
#include "Pi.h"
#include "WorldView.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Graphics.h"
#include "graphics/Texture.h"
#include "graphics/VertexArray.h"
#include "Color.h"

#ifdef _MSC_VER
	#include "win32/WinMath.h"
	#define log1pf LogOnePlusX
#endif // _MSC_VER

using namespace Graphics;

static const Graphics::AttributeSet RING_VERTEX_ATTRIBS
	= Graphics::ATTRIB_POSITION
	| Graphics::ATTRIB_UV0;

Planet::Planet(): TerrainBody(), m_ringVertices(RING_VERTEX_ATTRIBS)
{
}

Planet::Planet(SystemBody *sbody): TerrainBody(sbody), m_ringVertices(RING_VERTEX_ATTRIBS)
{
	InitParams(sbody);
}

Planet::~Planet() {}

void Planet::Load(Serializer::Reader &rd, Space *space)
{
	TerrainBody::Load(rd, space);

	const SystemBody *sbody = GetSystemBody();
	assert(sbody);
	InitParams(sbody);
}

void Planet::InitParams(const SystemBody *sbody)
{
	const double SPECIFIC_HEAT_AIR_CP=1000.5;// constant pressure specific heat, for the combination of gasses that make up air
	// XXX using earth's molar mass of air...
	const double GAS_MOLAR_MASS = 0.02897;
	const double GAS_CONSTANT = 8.3144621;
	const double PA_2_ATMOS = 1.0 / 101325.0;

	// surface gravity = -G*M/planet radius^2
	m_surfaceGravity_g = -G*sbody->GetMass()/(sbody->GetRadius()*sbody->GetRadius());
	const double lapseRate_L = -m_surfaceGravity_g/SPECIFIC_HEAT_AIR_CP; // negative deg/m
	const double surfaceTemperature_T0 = sbody->averageTemp; //K

	double surfaceDensity, h; Color c;
	sbody->GetAtmosphereFlavor(&c, &surfaceDensity);// kg / m^3
	surfaceDensity/=GAS_MOLAR_MASS;			// convert to moles/m^3

	//P = density*R*T=(n/V)*R*T
	const double surfaceP_p0 = PA_2_ATMOS*((surfaceDensity)*GAS_CONSTANT*surfaceTemperature_T0); // in atmospheres
	if (surfaceP_p0 < 0.002) h = 0;
	else {
		//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
		// want height for pressure 0.001 atm:
		// h = (1 - exp(RL/gM * log(P/p0))) * T0 / l
		double RLdivgM = (GAS_CONSTANT*lapseRate_L)/(-m_surfaceGravity_g*GAS_MOLAR_MASS);
		h = (1.0 - exp(RLdivgM * log(0.001/surfaceP_p0))) * surfaceTemperature_T0 / lapseRate_L;
//		double h2 = (1.0 - pow(0.001/surfaceP_p0, RLdivgM)) * surfaceTemperature_T0 / lapseRate_L;
//		double P = surfaceP_p0*pow((1.0-lapseRate_L*h/surfaceTemperature_T0),1/RLdivgM);
	}
	m_atmosphereRadius = h + sbody->GetRadius();

	SetPhysRadius(std::max(m_atmosphereRadius, GetMaxFeatureRadius()+1000));
	if (sbody->HasRings()) {
		SetClipRadius(sbody->GetRadius() * sbody->m_rings.maxRadius.ToDouble());
	} else {
		SetClipRadius(GetPhysRadius());
	}
}

/*
 * dist = distance from centre
 * returns pressure in earth atmospheres
 * function is slightly different from the isothermal earth-based approximation used in shaders,
 * but it isn't visually noticeable.
 */
void Planet::GetAtmosphericState(double dist, double *outPressure, double *outDensity) const
{
	PROFILE_SCOPED()
#if 0
	static bool atmosphereTableShown = false;
	if (!atmosphereTableShown) {
		atmosphereTableShown = true;
		for (double h = -1000; h <= 50000; h = h+1000.0) {
			double p = 0.0, d = 0.0;
			GetAtmosphericState(h+this->GetSystemBody()->GetRadius(),&p,&d);
			printf("height(m): %f, pressure(kpa): %f, density: %f\n", h, p*101325.0/1000.0, d);
		}
	}
#endif

	// This model has no atmosphere beyond the adiabetic limit
	// Note: some code duplicated in InitParams(). Check if changing.
	if (dist >= m_atmosphereRadius) {*outDensity = 0.0; *outPressure = 0.0; return;}

	double surfaceDensity;
	const double SPECIFIC_HEAT_AIR_CP=1000.5;// constant pressure specific heat, for the combination of gasses that make up air
	// XXX using earth's molar mass of air...
	const double GAS_MOLAR_MASS = 0.02897;
	const double GAS_CONSTANT = 8.3144621;
	const double PA_2_ATMOS = 1.0 / 101325.0;

	// lapse rate http://en.wikipedia.org/wiki/Adiabatic_lapse_rate#Dry_adiabatic_lapse_rate
	// the wet adiabatic rate can be used when cloud layers are incorporated
	// fairly accurate in the troposphere
	const double lapseRate_L = -m_surfaceGravity_g/SPECIFIC_HEAT_AIR_CP; // negative deg/m

	const SystemBody *sbody = this->GetSystemBody();
	const double height_h = (dist-sbody->GetRadius()); // height in m
	const double surfaceTemperature_T0 = sbody->averageTemp; //K

	Color c;
	sbody->GetAtmosphereFlavor(&c, &surfaceDensity);// kg / m^3
	// convert to moles/m^3
	surfaceDensity/=GAS_MOLAR_MASS;

	//P = density*R*T=(n/V)*R*T
	const double surfaceP_p0 = PA_2_ATMOS*((surfaceDensity)*GAS_CONSTANT*surfaceTemperature_T0); // in atmospheres

	// height below zero should not occur
	if (height_h < 0.0) { *outPressure = surfaceP_p0; *outDensity = surfaceDensity*GAS_MOLAR_MASS; return; }

	//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
	*outPressure = surfaceP_p0*pow((1-lapseRate_L*height_h/surfaceTemperature_T0),(-m_surfaceGravity_g*GAS_MOLAR_MASS/(GAS_CONSTANT*lapseRate_L)));// in ATM since p0 was in ATM
	//                                                                               ^^g used is abs(g)
	// temperature at height
	double temp = surfaceTemperature_T0+lapseRate_L*height_h;

	*outDensity = (*outPressure/(PA_2_ATMOS*GAS_CONSTANT*temp))*GAS_MOLAR_MASS;
}

void Planet::GenerateRings(Graphics::Renderer *renderer)
{
	const SystemBody *sbody = GetSystemBody();

	m_ringVertices.Clear();

	// generate the ring geometry
	const float inner = sbody->m_rings.minRadius.ToFloat();
	const float outer = sbody->m_rings.maxRadius.ToFloat();
	int segments = 200;
	for (int i = 0; i <= segments; ++i) {
		const float a = (2.0f*float(M_PI)) * (float(i) / float(segments));
		const float ca = cosf(a);
		const float sa = sinf(a);
		m_ringVertices.Add(vector3f(inner*sa, 0.0f, inner*ca), vector2f(float(i), 0.0f));
		m_ringVertices.Add(vector3f(outer*sa, 0.0f, outer*ca), vector2f(float(i), 1.0f));
	}

	// generate the ring texture
	// NOTE: texture width must be > 1 to avoid graphical glitches with Intel GMA 900 systems
	//       this is something to do with mipmapping (probably mipmap generation going wrong)
	//       (if the texture is generated without mipmaps then a 1xN texture works)
	const int RING_TEXTURE_WIDTH = 4;
	const int RING_TEXTURE_LENGTH = 256;
	std::unique_ptr<Color, FreeDeleter> buf(
			static_cast<Color*>(malloc(RING_TEXTURE_WIDTH * RING_TEXTURE_LENGTH * 4)));

	const float ringScale = (outer-inner)*sbody->GetRadius() / 1.5e7f;

	Random rng(GetSystemBody()->seed+4609837);
	Color baseCol = sbody->m_rings.baseColor;
	double noiseOffset = 2048.0 * rng.Double();
	for (int i = 0; i < RING_TEXTURE_LENGTH; ++i) {
		const float alpha = (float(i) / float(RING_TEXTURE_LENGTH)) * ringScale;
		const float n = 0.25 +
			0.60 * noise( 5.0 * alpha, noiseOffset, 0.0) +
			0.15 * noise(10.0 * alpha, noiseOffset, 0.0);

		const float LOG_SCALE = 1.0f/sqrtf(sqrtf(log1pf(1.0f)));
		const float v = LOG_SCALE*sqrtf(sqrtf(log1pf(n)));

		Color color;
		color.r = v*baseCol.r;
		color.g = v*baseCol.g;
		color.b = v*baseCol.b;
		color.a = ((v*0.25f)+0.75f)*baseCol.a;

		Color *row = buf.get() + i * RING_TEXTURE_WIDTH;
		for (int j = 0; j < RING_TEXTURE_WIDTH; ++j) {
			row[j] = color;
		}
	}

	// first and last pixel are forced to zero, to give a slightly smoother ring edge
	{
		Color *row;
		row = buf.get();
		memset(row, 0, RING_TEXTURE_WIDTH * 4);
		row = buf.get() + (RING_TEXTURE_LENGTH - 1) * RING_TEXTURE_WIDTH;
		memset(row, 0, RING_TEXTURE_WIDTH * 4);
	}

	const vector2f texSize(RING_TEXTURE_WIDTH, RING_TEXTURE_LENGTH);
	const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888, texSize, Graphics::LINEAR_REPEAT, true);

	m_ringTexture.Reset(renderer->CreateTexture(texDesc));
	m_ringTexture->Update(
			static_cast<void*>(buf.get()), texSize,
			Graphics::TEXTURE_RGBA_8888);

	Graphics::MaterialDescriptor desc;
	desc.effect = Graphics::EFFECT_PLANETRING;
	desc.lighting = true;
	desc.twoSided = true;
	desc.textures = 1;
	m_ringMaterial.reset(renderer->CreateMaterial(desc));
	m_ringMaterial->texture0 = m_ringTexture.Get();
}

void Planet::DrawGasGiantRings(Renderer *renderer, const matrix4x4d &modelView)
{
	renderer->SetBlendMode(BLEND_ALPHA_PREMULT);
	renderer->SetDepthTest(true);

	if (!m_ringTexture)
		GenerateRings(renderer);

	const SystemBody *sbody = GetSystemBody();
	assert(sbody->HasRings());

	renderer->SetTransform(modelView);

	renderer->DrawTriangles(&m_ringVertices, m_ringMaterial.get(), TRIANGLE_STRIP);

	renderer->SetBlendMode(BLEND_SOLID);
}

void Planet::DrawAtmosphere(Renderer *renderer, const matrix4x4d &modelView, const vector3d &camPos)
{
	PROFILE_SCOPED()
	//this is the non-shadered atmosphere rendering
	Color col;
	double density;
	GetSystemBody()->GetAtmosphereFlavor(&col, &density);

	const double rad1 = 0.999;
	const double rad2 = 1.05;

	// face the camera dammit
	vector3d zaxis = (-camPos).Normalized();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
	const matrix4x4d trans = modelView * rot;

	matrix4x4d invViewRot = trans;
	invViewRot.ClearToRotOnly();
	invViewRot = invViewRot.InverseOf();

	// XXX used to be Pi::worldView->GetNumLights, but that always returns 1
	const int numLights = 1;
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

	if (!m_atmosphereVertices) {
		m_atmosphereVertices.reset(new Graphics::VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE | ATTRIB_NORMAL));
		Graphics::MaterialDescriptor desc;
		desc.vertexColors = true;
		desc.twoSided = true;
		m_atmosphereMaterial.reset(renderer->CreateMaterial(desc));
	}

	VertexArray &vts = *m_atmosphereVertices;
	vts.Clear();

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
		vts.Add(vector3f(r2.x, r2.y, r2.z), Color(0), n);
		r1 = rot * r1;
		r2 = rot * r2;
	}

	renderer->SetTransform(trans);
	renderer->SetBlendMode(BLEND_ALPHA_ONE);
	renderer->DrawTriangles(m_atmosphereVertices.get(), m_atmosphereMaterial.get(), TRIANGLE_STRIP);
	renderer->SetBlendMode(BLEND_SOLID);
}

void Planet::SubRender(Renderer *r, const matrix4x4d &viewTran, const vector3d &camPos)
{
	if (GetSystemBody()->HasRings()) { DrawGasGiantRings(r, viewTran); }
}
