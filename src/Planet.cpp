// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Planet.h"

#include "Color.h"
#include "GeoSphere.h"
#include "galaxy/SystemBody.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "perlin.h"

#ifdef _MSC_VER
#include "win32/WinMath.h"
#endif // _MSC_VER

using namespace Graphics;

static const Graphics::AttributeSet RING_VERTEX_ATTRIBS = Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0;

Planet::Planet(SystemBody *sbody) :
	TerrainBody(sbody),
	m_ringVertices(RING_VERTEX_ATTRIBS),
	m_ringState(nullptr)
{
	InitParams(sbody);
}

Planet::Planet(const Json &jsonObj, Space *space) :
	TerrainBody(jsonObj, space),
	m_ringVertices(RING_VERTEX_ATTRIBS),
	m_ringState(nullptr)
{
	const SystemBody *sbody = GetSystemBody();
	assert(sbody);
	InitParams(sbody);
}

void Planet::InitParams(const SystemBody *sbody)
{
	double specificHeatCp;
	double gasMolarMass;
	if (sbody->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT) {
		specificHeatCp = 12950.0; // constant pressure specific heat, for a combination of hydrogen and helium
		gasMolarMass = 0.0023139903;
	} else {
		specificHeatCp = 1000.5; // constant pressure specific heat, for the combination of gasses that make up air
		// XXX using earth's molar mass of air...
		gasMolarMass = 0.02897;
	}
	const double GAS_CONSTANT = 8.3144621;
	const double PA_2_ATMOS = 1.0 / 101325.0;

	// surface gravity = -G*M/planet radius^2
	m_surfaceGravity_g = -G * sbody->GetMass() / (sbody->GetRadius() * sbody->GetRadius());
	const double lapseRate_L = -m_surfaceGravity_g / specificHeatCp; // negative deg/m
	const double surfaceTemperature_T0 = sbody->GetAverageTemp(); //K

	double surfaceDensity, h;
	Color c;
	sbody->GetAtmosphereFlavor(&c, &surfaceDensity); // kg / m^3
	surfaceDensity /= gasMolarMass; // convert to moles/m^3

	//P = density*R*T=(n/V)*R*T
	const double surfaceP_p0 = PA_2_ATMOS * ((surfaceDensity)*GAS_CONSTANT * surfaceTemperature_T0); // in atmospheres
	if (surfaceP_p0 < 0.002)
		h = 0;
	else {
		//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
		// want height for pressure 0.001 atm:
		// h = (1 - exp(RL/gM * log(P/p0))) * T0 / l
		double RLdivgM = (GAS_CONSTANT * lapseRate_L) / (-m_surfaceGravity_g * gasMolarMass);
		h = (1.0 - exp(RLdivgM * log(0.001 / surfaceP_p0))) * surfaceTemperature_T0 / lapseRate_L;
		//		double h2 = (1.0 - pow(0.001/surfaceP_p0, RLdivgM)) * surfaceTemperature_T0 / lapseRate_L;
		//		double P = surfaceP_p0*pow((1.0-lapseRate_L*h/surfaceTemperature_T0),1/RLdivgM);
	}
	m_atmosphereRadius = h + sbody->GetRadius();

	SetPhysRadius(std::max(m_atmosphereRadius, GetMaxFeatureRadius() + 1000));
	// NB: Below abandoned due to docking problems with low altitude orbiting space stations
	// SetPhysRadius(std::max(m_atmosphereRadius, std::max(GetMaxFeatureRadius() * 2.0 + 2000, sbody->GetRadius() * 1.05)));
	if (sbody->HasRings()) {
		SetClipRadius(sbody->GetRadius() * sbody->GetRings().maxRadius.ToDouble());
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
		for (double h = -1000; h <= 100000; h = h+1000.0) {
			double p = 0.0, d = 0.0;
			GetAtmosphericState(h+this->GetSystemBody()->GetRadius(),&p,&d);
			Output("height(m): %f, pressure(hpa): %f, density: %f\n", h, p*101325.0/100.0, d);
		}
	}
#endif

	// This model has no atmosphere beyond the adiabetic limit
	// Note: some code duplicated in InitParams(). Check if changing.
	if (dist >= m_atmosphereRadius) {
		*outDensity = 0.0;
		*outPressure = 0.0;
		return;
	}

	double surfaceDensity;
	double specificHeatCp;
	double gasMolarMass;
	const SystemBody *sbody = this->GetSystemBody();
	if (sbody->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT) {
		specificHeatCp = 12950.0; // constant pressure specific heat, for a combination of hydrogen and helium
		gasMolarMass = 0.0023139903;
	} else {
		specificHeatCp = 1000.5; // constant pressure specific heat, for the combination of gasses that make up air
		// XXX using earth's molar mass of air...
		gasMolarMass = 0.02897;
	}
	const double GAS_CONSTANT = 8.3144621;
	const double PA_2_ATMOS = 1.0 / 101325.0;

	// lapse rate http://en.wikipedia.org/wiki/Adiabatic_lapse_rate#Dry_adiabatic_lapse_rate
	// the wet adiabatic rate can be used when cloud layers are incorporated
	// fairly accurate in the troposphere
	const double lapseRate_L = -m_surfaceGravity_g / specificHeatCp; // negative deg/m

	const double height_h = (dist - sbody->GetRadius()); // height in m
	const double surfaceTemperature_T0 = sbody->GetAverageTemp(); //K

	Color c;
	sbody->GetAtmosphereFlavor(&c, &surfaceDensity); // kg / m^3
	// convert to moles/m^3
	surfaceDensity /= gasMolarMass;

	//P = density*R*T=(n/V)*R*T
	const double surfaceP_p0 = PA_2_ATMOS * ((surfaceDensity)*GAS_CONSTANT * surfaceTemperature_T0); // in atmospheres

	// height below zero should not occur
	if (height_h < 0.0) {
		*outPressure = surfaceP_p0;
		*outDensity = surfaceDensity * gasMolarMass;
		return;
	}

	//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
	*outPressure = surfaceP_p0 * pow((1 - lapseRate_L * height_h / surfaceTemperature_T0), (-m_surfaceGravity_g * gasMolarMass / (GAS_CONSTANT * lapseRate_L))); // in ATM since p0 was in ATM
	//                                                                               ^^g used is abs(g)
	// temperature at height
	double temp = surfaceTemperature_T0 + lapseRate_L * height_h;

	*outDensity = (*outPressure / (PA_2_ATMOS * GAS_CONSTANT * temp)) * gasMolarMass;
}

void Planet::GenerateRings(Graphics::Renderer *renderer)
{
	const SystemBody *sbody = GetSystemBody();

	m_ringVertices.Clear();

	// generate the ring geometry
	const float inner = sbody->GetRings().minRadius.ToFloat();
	const float outer = sbody->GetRings().maxRadius.ToFloat();
	int segments = 200;
	for (int i = 0; i <= segments; ++i) {
		const float a = (2.0f * float(M_PI)) * (float(i) / float(segments));
		const float ca = cosf(a);
		const float sa = sinf(a);
		m_ringVertices.Add(vector3f(inner * sa, 0.0f, inner * ca), vector2f(float(i), 0.0f));
		m_ringVertices.Add(vector3f(outer * sa, 0.0f, outer * ca), vector2f(float(i), 1.0f));
	}

	// generate the ring texture
	// NOTE: texture width must be > 1 to avoid graphical glitches with Intel GMA 900 systems
	//       this is something to do with mipmapping (probably mipmap generation going wrong)
	//       (if the texture is generated without mipmaps then a 1xN texture works)
	const int RING_TEXTURE_WIDTH = 4;
	const int RING_TEXTURE_LENGTH = 256;
	std::unique_ptr<Color, FreeDeleter> buf(
		static_cast<Color *>(malloc(RING_TEXTURE_WIDTH * RING_TEXTURE_LENGTH * 4)));

	const float ringScale = (outer - inner) * sbody->GetRadius() / 1.5e7f;

	Random rng(GetSystemBody()->GetSeed() + 4609837);
	Color baseCol = sbody->GetRings().baseColor;
	double noiseOffset = 2048.0 * rng.Double();
	for (int i = 0; i < RING_TEXTURE_LENGTH; ++i) {
		const float alpha = (float(i) / float(RING_TEXTURE_LENGTH)) * ringScale;
		const float n = 0.25 +
			0.60 * noise(vector3d(5.0 * alpha, noiseOffset, 0.0)) +
			0.15 * noise(vector3d(10.0 * alpha, noiseOffset, 0.0));

		const float LOG_SCALE = 1.0f / sqrtf(sqrtf(log1p(1.0f)));
		const float v = LOG_SCALE * sqrtf(sqrtf(log1p(n)));

		Color color;
		color.r = v * baseCol.r;
		color.g = v * baseCol.g;
		color.b = v * baseCol.b;
		color.a = ((v * 0.25f) + 0.75f) * baseCol.a;

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

	const vector3f texSize(RING_TEXTURE_WIDTH, RING_TEXTURE_LENGTH, 0.0f);
	const Graphics::TextureDescriptor texDesc(
		Graphics::TEXTURE_RGBA_8888, texSize, Graphics::LINEAR_REPEAT, true, true, true, 0, Graphics::TEXTURE_2D);

	m_ringTexture.Reset(renderer->CreateTexture(texDesc));
	m_ringTexture->Update(
		static_cast<void *>(buf.get()), texSize,
		Graphics::TEXTURE_RGBA_8888);

	Graphics::MaterialDescriptor desc;
	desc.effect = Graphics::EFFECT_PLANETRING;
	desc.lighting = true;
	desc.textures = 1;
	m_ringMaterial.reset(renderer->CreateMaterial(desc));
	m_ringMaterial->texture0 = m_ringTexture.Get();

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_PREMULT;
	rsd.cullMode = Graphics::CULL_NONE;
	m_ringState = renderer->CreateRenderState(rsd);
}

void Planet::DrawGasGiantRings(Renderer *renderer, const matrix4x4d &modelView)
{
	assert(GetSystemBody()->HasRings());

	if (!m_ringTexture)
		GenerateRings(renderer);

	renderer->SetTransform(modelView);
	renderer->DrawTriangles(&m_ringVertices, m_ringState, m_ringMaterial.get(), TRIANGLE_STRIP);
}

void Planet::SubRender(Renderer *r, const matrix4x4d &viewTran, const vector3d &camPos)
{
	if (GetSystemBody()->HasRings()) {
		DrawGasGiantRings(r, viewTran);
	}
}
