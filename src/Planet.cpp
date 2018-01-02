// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Planet.h"
#include "Pi.h"
#include "WorldView.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/RenderState.h"
#include "graphics/Graphics.h"
#include "graphics/Texture.h"
#include "graphics/VertexArray.h"
#include "Color.h"

#ifdef _MSC_VER
	#include "win32/WinMath.h"
#endif // _MSC_VER

using namespace Graphics;

static const Graphics::AttributeSet RING_VERTEX_ATTRIBS
	= Graphics::ATTRIB_POSITION
	| Graphics::ATTRIB_UV0;

Planet::Planet()
	: TerrainBody()
	, m_ringVertices(RING_VERTEX_ATTRIBS)
	, m_ringState(nullptr)
{
}

Planet::Planet(SystemBody *sbody)
	: TerrainBody(sbody)
	, m_ringVertices(RING_VERTEX_ATTRIBS)
	, m_ringState(nullptr)
{
	InitParams(sbody);
}

void Planet::LoadFromJson(const Json::Value &jsonObj, Space *space)
{
	TerrainBody::LoadFromJson(jsonObj, space);

	const SystemBody *sbody = GetSystemBody();
	assert(sbody);
	InitParams(sbody);
}

void Planet::InitParams(const SystemBody *sbody)
{
	// All bodies have gravity
	// surface gravity = -G*M/planet radius^2
	m_surfaceGravity_g = -G * sbody->GetMass() / (sbody->GetRadius()*sbody->GetRadius());

	Color c; // value not used but function call requires a buffer to fill
	double surfaceDensity;

	// function returns surface atmosphere density in kg per cubic meter (m^3)
	sbody->GetAtmosphereFlavor(&c, &surfaceDensity);

	// if surfaceDensity is not zero, calculate an atmosphere
	if (surfaceDensity > 0)
	{
		double atmosphereheight, specificHeatCp, gasMolarMass;
		static const double GAS_CONSTANT = 8.3144598; // as defined by https://en.wikipedia.org/wiki/Gas_constant
		static const double PASCAL_PER_ATMOSPHERE = 101325; // 1 Atmosphere of pressure = 101325 Pascal

		// molar mass of hydrogen gas = 2.016g/mol
		// molar mass of helium gas   = 4.002g/mol
		// molar mass of earth air    = 28.97g/mol
		if (sbody->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT)
		{
			specificHeatCp = 12950.0; // constant pressure specific heat, for a combination of hydrogen and helium
			gasMolarMass = 2.3139;    // molar mass of hydrogen gas(85%) and helium(15%) mix = 2.3139g/mol
		}
		else
		{
			specificHeatCp = 1000.5;  // constant pressure specific heat, for the combination of gasses that make up air
			gasMolarMass = 28.97;     // grams/mol (earth air)
		}

		// get surface temperature in degrees Kelvin
		const double surfaceTemperature = sbody->GetAverageTemp();

		// https://en.wikipedia.org/wiki/Gas_constant#Specific_gas_constant
		// Rspecific = GAS_CONSTANT / M (molar mass)
		// Wikipedia uses a gram GAS_CONSTANT, ours is in kg, so these results are effectively divided by 1000.
		// Wikipedia states: Rspecific for dry air = 287.058, calculated here to 0.287002 for earth air and 3.593266 for gas giant
		const double Rspecific = GAS_CONSTANT / gasMolarMass;

		// Ideal gas law formula uses grams per cubic meter, not kilograms. Multiply by 1000.
		surfaceDensity *= 1000;

		// Calculate surface pressure using https://en.wikipedia.org/wiki/Ideal_gas_law
		// P (pressure in Pascal) = density (in grams per cubic meter) * Rspecific * temperature (in degrees Kelvin)
		const double surfacePressure = surfaceDensity * Rspecific * surfaceTemperature;
		// Earth values: surfacePressure 1225 g/m^3 * Rspecific 0.278002 * temperature 288K = 101254 Pascal (0.9993 atmospheres)
		// temperature of 288.2K gives 1 atmosphere exactly

		// so, whats the result? more than 0.002 atmospheres? 1 atmosphere is 101325 Pascal
		if (surfacePressure < (0.002 / PASCAL_PER_ATMOSPHERE))
			atmosphereheight = 0; // nope. surface pressure is below 0.002 atmospheres. no atmosphere for this little boy!
		else
		{
			// https://en.wikipedia.org/wiki/Lapse_rate
			// lapserate is the change in temperature as altitude increases, in degrees Kelvin per meter
			const double lapseRate = -m_surfaceGravity_g / specificHeatCp;

			// The formula used below is unknown, but here are some references
			// https://en.wikipedia.org/wiki/Vertical_pressure_variation
			// https://en.wikipedia.org/wiki/Density_of_air
			// it is likely that the formula used below is taken out of context and thats why it produces poor results

			{
				// this is a mess, something better is needed. original comments preserved.
				// calculation slightly adjusted but verified to match unaltered old version.

				//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
				// want height for pressure 0.001 atm:
				// h = (1 - exp(RL/gM * log(P/p0))) * T0 / l
				double RLdivgM = (GAS_CONSTANT*lapseRate) / (-m_surfaceGravity_g * (gasMolarMass / 1000));
				atmosphereheight = (1.0 - exp(RLdivgM * log(0.001 / (surfacePressure / PASCAL_PER_ATMOSPHERE)))) * surfaceTemperature / lapseRate;
				//double h2 = (1.0 - pow(0.001/surfaceP_p0, RLdivgM)) * surfaceTemperature_T0 / lapseRate_L;
				//double P = surfaceP_p0*pow((1.0-lapseRate_L*h/surfaceTemperature_T0),1/RLdivgM);
			}
			// by the above formula, earths atmosphere ends at 25344 meters.
		}
		m_atmosphereRadius = atmosphereheight + sbody->GetRadius();

		// 1 Pascal is 980665 kg/m2
		// surfacePressure * 980655 = total mass in of the pillar of air above you
		// divided by the density = how high the pillar needs to be to weigh that much at constant density
		//
		// this is not a formula based in science, but it gives surprisingly good/usable results, much better than the complicated formula above
		//
		// examples:		Planet			Old formula			testformula				reality
		//					Venus			80km				206km					~250km
		//					Earth			25km				80km					~100km
		//					Mars			62km				78km					?
		//					Jupiter			79km				581km					50km?
		//					Io				46km				36km									<-- in game 8271 Pa pressure (systems/00_sol.lua), in reality <0.0003 Pa
		//					Europa			47km				28km									<-- in game 7093 Pa pressure (systems/00_sol.lua), in reality 0.000001 Pa
		//					Rhea			17403km				22km									<-- this shows how absurd the old formula can get
		//					Icarus			25km				193km									<-- icarus is very hot (+414C), heat raises pressure, this might need to be taken into account
		//
		// when it comes to temperature, you can calculate the escape velocity of a body,
		// and you can calculate the actual velocity of a gas at a given temperature
		// https://en.wikipedia.org/wiki/Atmospheric_escape reading material
		//
		double testformula = surfacePressure * 980.665 / surfaceDensity;

		Output("Planet::InitParams> Planet `%s', type = %s, surfaceDensity %.0f g/m^3, surfaceTemperature %.1fK, Rspecific %.3f,\n"
			"                    surfacePressure %.0f Pascal (%.2f atmospheres), atmosphereheight %.0f meters, testformula = %.0f meters\n",
			sbody->GetName(), ((sbody->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT) ? "gas giant" : "rocky body"),
			surfaceDensity, surfaceTemperature, Rspecific, surfacePressure, (surfacePressure / PASCAL_PER_ATMOSPHERE), atmosphereheight, testformula);
	}
	else
	{
		// no surfacePressure = no atmosphere
		m_atmosphereRadius = sbody->GetRadius();
	}

	SetPhysRadius(std::max(m_atmosphereRadius, GetMaxFeatureRadius()+1000));
	// NB: Below abandoned due to docking problems with low altitude orbiting space stations
	// SetPhysRadius(std::max(m_atmosphereRadius, std::max(GetMaxFeatureRadius() * 2.0 + 2000, sbody->GetRadius() * 1.05)));
	if (sbody->HasRings())
	{
		SetClipRadius(sbody->GetRadius() * sbody->GetRings().maxRadius.ToDouble());
	}
	else
	{
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
	if (dist >= m_atmosphereRadius) {*outDensity = 0.0; *outPressure = 0.0; return;}

	// molar mass of hydrogen gas = 2.016g/mol
	// molar mass of helium gas = 4.002g/mol
	// molar mass of earth air = 28.97g/mol
	// molar mass of hydrogen gas(85%) and helium(15%) mix = 2.3139g/mol

	double surfaceDensity;
	double specificHeatCp;
	double gasMolarMass;
	const SystemBody *sbody = this->GetSystemBody();
	if (sbody->GetSuperType() == SystemBody::SUPERTYPE_GAS_GIANT) {
		specificHeatCp=12950.0; // constant pressure specific heat, for a combination of hydrogen and helium
		gasMolarMass = 0.0023139903;
	} else {
		specificHeatCp=1000.5;// constant pressure specific heat, for the combination of gasses that make up air
		// XXX using earth's molar mass of air...
		gasMolarMass = 0.02897;
	}
	const double GAS_CONSTANT = 8.3144621;
	const double PA_2_ATMOS = 1.0 / 101325.0;

	// lapse rate http://en.wikipedia.org/wiki/Adiabatic_lapse_rate#Dry_adiabatic_lapse_rate
	// the wet adiabatic rate can be used when cloud layers are incorporated
	// fairly accurate in the troposphere
	// m_surfaceGravity_g is calculated and stored as a negative number
	const double lapseRate_L = -m_surfaceGravity_g/specificHeatCp; // negative deg/m << wrong? -(-)/+ = +, should it be -abs(G)/hcp?
	const double height_h = (dist-sbody->GetRadius()); // height in m
	const double surfaceTemperature_T0 = sbody->GetAverageTemp(); //K

	Color c;
	sbody->GetAtmosphereFlavor(&c, &surfaceDensity);// kg / m^3
	// convert to moles/m^3
	surfaceDensity/=gasMolarMass;

	//P = density*R*T=(n/V)*R*T
	const double surfaceP_p0 = PA_2_ATMOS*((surfaceDensity)*GAS_CONSTANT*surfaceTemperature_T0); // in atmospheres

	// height below zero should not occur
	if (height_h < 0.0) { *outPressure = surfaceP_p0; *outDensity = surfaceDensity*gasMolarMass; return; }

	//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
	// WARNING! lapserate is a positive number, not negative as commented
	*outPressure = surfaceP_p0*pow((1-lapseRate_L*height_h/surfaceTemperature_T0),(-m_surfaceGravity_g*gasMolarMass/(GAS_CONSTANT*lapseRate_L)));// in ATM since p0 was in ATM
	//                                                                               ^^g used is abs(g)
	// temperature at height
	double temp = surfaceTemperature_T0-lapseRate_L*height_h; // << lapserate is a positive number and needs to be subtracted for temperature to drop the higher you get.

	int th = floor(height_h);
	if ((th % 1000) == 0)
		Output("Atmosphere at %.0f meters, temperature = %.1fC, with + temperature (unfixed) = %.1fC\n", height_h, temp - 273, (surfaceTemperature_T0 + lapseRate_L * height_h) - 273);

	*outDensity = (*outPressure/(PA_2_ATMOS*GAS_CONSTANT*temp))*gasMolarMass;
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

	Random rng(GetSystemBody()->GetSeed()+4609837);
	Color baseCol = sbody->GetRings().baseColor;
	double noiseOffset = 2048.0 * rng.Double();
	for (int i = 0; i < RING_TEXTURE_LENGTH; ++i) {
		const float alpha = (float(i) / float(RING_TEXTURE_LENGTH)) * ringScale;
		const float n = 0.25 +
			0.60 * noise(vector3d( 5.0 * alpha, noiseOffset, 0.0)) +
			0.15 * noise(vector3d(10.0 * alpha, noiseOffset, 0.0));

		const float LOG_SCALE = 1.0f/sqrtf(sqrtf(log1p(1.0f)));
		const float v = LOG_SCALE*sqrtf(sqrtf(log1p(n)));

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
			Graphics::TEXTURE_RGBA_8888, texSize, Graphics::LINEAR_REPEAT, true, true, true, 0, Graphics::TEXTURE_2D);

	m_ringTexture.Reset(renderer->CreateTexture(texDesc));
	m_ringTexture->Update(
			static_cast<void*>(buf.get()), texSize,
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
	if (GetSystemBody()->HasRings()) { DrawGasGiantRings(r, viewTran); }
}
