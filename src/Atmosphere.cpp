// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Atmosphere.h"

/*
*
* Gas definitions - These contain all the gas specific properies 
*                   needed for the atmosphere (both physics and rendering)
* To extend       - 1. Copy the template below, and fill in values. 
*                   2. Add definition to Atmosphere class. Similarly for Aerosols.
*
/*
const Gas Atmosphere::s_gasnamehere(
	"Name here",
	SPECIFIC_HEAT_OF_GAS,
	MOLAR_MASS_OF_GAS,
	RAYLIEGH CONSTANT,
	Color(0.5, 0.0, 0.5, 1.0)
);
*/
// Temporary values based on Earth atmosphere
static const double MOLAR_MASS_OF_EARTH_ATMOSPHERE = 0.02897; // kg/mol
static const double SPECIFIC_HEAT_OF_EARTH_AIR = 1000.5;      // J/kg/K
static const double PLACEHOLDER = 1.0;                        // placeholder

/*
// Ar
	r = 0.5f - ((0.15f-atmo)*5.0);
	g = 0.0f;
	b = 0.5f + ((0.15f-atmo)*5.0);
*/

const Gas Atmosphere::s_argon(
	"Argon",
	520,
	0.040,
	PLACEHOLDER,
	Color(0.5, 0.0, 0.5, 1.0)
);

/*
// CO2
	r = 0.05f+atmo;
	g = 1.0f + (0.7f-atmo);
	b = 0.8f;
*/
const Gas Atmosphere::s_carbonDioxide(
	"CarbonDioxide",
	844,
	0.04401,
	PLACEHOLDER,
	Color(0.05, 1.0, 0.8, 1.0)
);
/*
// CO
	r = 1.0f + (0.65f-atmo);
	g = 0.8f;
	b = 0.25f+atmo;
*/
const Gas Atmosphere::s_carbonMonoxide(
	"Carbon Monoxide",
	1020,
	0.02801,
	PLACEHOLDER,
	Color(1.0, 0.8, 0.25, 1.0)
);
/*
// He
	r = 1.0f;
	g = 1.0f;
	b = 1.0f;
*/

const Gas Atmosphere::s_helium(
	"Helium",
	5190,
	0.002,
	PLACEHOLDER,
	Color(1.0, 1.0, 1.0, 1.0)
);
/*
// H
	r = 1.0f;
	g = 1.0f;
	b = 1.0f;
*/

const Gas Atmosphere::s_hydrogen(
	"Hydrogen",
	13120,
	0.002,
	PLACEHOLDER,
	Color(1.0, 1.0, 1.0, 1.0)
);
/*
// O2
	r = 1.0f + ((0.95f-atmo)*15.0f);
	g = 0.95f + ((0.95f-atmo)*10.0f);
	b = atmo*atmo*atmo*atmo*atmo;
*/
const Gas Atmosphere::s_oxygen(
	"Oxygen",
	SPECIFIC_HEAT_OF_EARTH_AIR,
	MOLAR_MASS_OF_EARTH_ATMOSPHERE,
	PLACEHOLDER,
	Color(1.0, 0.95, 0.0, 1.0)
);
/*
// CH4
	r = 1.0f + ((0.55f-atmo)*5.0);
	g = 0.35f - ((0.55f-atmo)*5.0);
	b = 0.4f;
*/

const Gas Atmosphere::s_methane(
	"Methane",
	2220,
	0.016,
	PLACEHOLDER,
	Color(1.0, 0.35, 0.4, 1.0)
);
/*
// N
	r = 1.0f;
	g = 1.0f;
	b = 1.0f;
*/

const Gas Atmosphere::s_nitrogen(
	"Nitrogen",
	1035,
	0.028,
	PLACEHOLDER,
	Color(1.0, 1.0, 1.0, 1.0)
);
/*
// SO2
	r = 0.8f - ((0.1f-atmo)*4.0);
	g = 1.0f;
	b = 0.5f - ((0.1f-atmo)*10.0);
*/

const Gas Atmosphere::s_sulfurDioxide(
	"Sulfur Dioxide",
	0.064,
	640,
	PLACEHOLDER,
	Color(0.8, 1.0, 0.5, 1.0)
);
const Gas Atmosphere::s_whiteGas(
	"White Gas",
	SPECIFIC_HEAT_OF_EARTH_AIR,
	MOLAR_MASS_OF_EARTH_ATMOSPHERE,
	PLACEHOLDER,
	Color(0.0, 0.0, 0.0, 1.0)
);
const Gas Atmosphere::s_earthAtmosphere(
	"Earth Atmosphere",
	SPECIFIC_HEAT_OF_EARTH_AIR,
	MOLAR_MASS_OF_EARTH_ATMOSPHERE,
	PLACEHOLDER,
	Color(1.0, 1.0, 1.0, 1.0)
);
const Gas Atmosphere::s_gasGiantAtmosphere(
	"Gas Giant atmosphere for single constituent model",
	SPECIFIC_HEAT_OF_EARTH_AIR,
	MOLAR_MASS_OF_EARTH_ATMOSPHERE,
	PLACEHOLDER,
	Color(1.0, 1.0, 1.0, 0.0005)
);
const Gas Atmosphere::s_vacuum(
	"Vacuum",
	SPECIFIC_HEAT_OF_EARTH_AIR,
	MOLAR_MASS_OF_EARTH_ATMOSPHERE,
	PLACEHOLDER,
	Color(0.0, 0.0, 0.0, 1.0)
);

/*
*
* End static gas definition 
*
*/

Atmosphere::Atmosphere(SystemBody *sbody)
{
	Init(sbody);
}

Atmosphere::~Atmosphere()
{
	// lazy evaluation is used to save computation when systems outside the
	// current one are scanned.
	// it's possible atmos params may not have been initialised
	if (m_shaderAtmosParams)
		delete m_shaderAtmosParams;
}

void Atmosphere::Init(SystemBody *sbody)
{
	m_sbody = sbody;
	m_atmosOxidizing = sbody->m_atmosOxidizing.ToDouble();
	m_shaderAtmosParams = 0;

	PickSingleConstituentAtmosphere(sbody);
	CalculateSimpleScatteringModelProperties();
}

void Atmosphere::CalculateGasConstituentsForSingleConstituentModel(SystemBody *sbody)
{
		double &atmo = m_atmosOxidizing;
		GasConstituent gc;
		const SystemBody::BodyType type = sbody->type;
		switch (type) {
			case SystemBody::TYPE_PLANET_GAS_GIANT:
				gc = GasConstituent(
					&s_hydrogen, 1.0,
					vector3d(0.0, 0.0, 0.0),
					vector3d(0.0, 0.0, 0.0)
				);
				//gc = g;
				break;
			case SystemBody::TYPE_PLANET_ASTEROID:
				gc = GasConstituent(
					&s_vacuum, 1.0,
					vector3d(0.0, 0.0, 0.0),
					vector3d(0.0, 0.0, 0.0)
				);
				break;
			default:
			case SystemBody::TYPE_PLANET_TERRESTRIAL:
		if (sbody->m_volatileGas.ToDouble() > 0.001) {
			// the rgb equations from the old code are in comments
			// the equation for each colour component is of the form:
			//    colour.r = 
			//	      gas->col.r + colChangeOffset.x + colChangeRange.y*atmosOxidising
			// and is calculated in GasConstituent::GetGasColour

			// col.r is already assigned to a member of the relevant static gas class
			// colChangeOffset and colChangeRange are assigned through the GasConstituents struct
			if (atmo > 0.95) {
				/*
				 O2
					r = 1.0f + ((0.95f-atmo)*15.0f);
					g = 0.95f + ((0.95f-atmo)*10.0f);
					b = atmo*atmo*atmo*atmo*atmo;
				*/
				gc = GasConstituent(
					&s_oxygen, 1.0,
					vector3d(0.95*15.0,  0.95*10.0, atmo*atmo*atmo*atmo*atmo),
					vector3d(-1.0*15.0,  -1.0*10.0, 0.0)
				);
			} else if (atmo > 0.7) {
				/*
				// CO2
					r = 0.05f+atmo;
					g = 1.0f + (0.7f-atmo);
					b = 0.8f;
				*/
				gc = GasConstituent(
					&s_carbonDioxide, 1.0,
					vector3d(0.0,  0.7, 0.0),
					vector3d(1.0, -1.0, 0.0)
				);
			} else if (atmo > 0.65) {
				/*
				// CO
					r = 1.0f + (0.65f-atmo);
					g = 0.8f;
					b = 0.25f + atmo;
				*/
				gc = GasConstituent(
					&s_carbonMonoxide, 1.0,
					vector3d(0.65,  0.0, 0.0),
					vector3d(-1.0,  0.0, 1.0)
				);
			} else if (atmo > 0.55) {
				/*
				// CH4
					r = 1.0f + ((0.55f-atmo)*5.0);
					g = 0.35f - ((0.55f-atmo)*5.0);
					b = 0.4f;
				*/
				gc = GasConstituent(
					&s_methane, 1.0,
					vector3d(0.55*5.0, -0.55*5.0, 0.0),
					vector3d(-1.0*5.0,  1.0*5.0,  0.0)
				);
			} else if (atmo > 0.3) {
				/*
				   H
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
				*/
				gc = GasConstituent(
					&s_hydrogen, 1.0,
					vector3d(0.0,  0.0, 0.0),
					vector3d(0.0,  0.0, 0.0)
				);
			} else if (atmo > 0.2) {
				/*
				// He
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
				*/
				gc = GasConstituent(
					&s_helium, 1.0,
					vector3d(0.0,  0.0, 0.0),
					vector3d(0.0,  0.0, 0.0)
				);
			} else if (atmo > 0.15) {
				/*
				// Ar
					r = 0.5f - ((0.15f-atmo)*5.0);
					g = 0.0f;
					b = 0.5f + ((0.15f-atmo)*5.0);
				*/
				gc = GasConstituent(
					&s_argon, 1.0,
					vector3d(-0.15*5.0, 0.0,  0.15*5.0),
					vector3d(1.0*5.0,   0.0, -1.0*5.0)
				);
			} else if (atmo > 0.1) {
				/*
				// SO2
					r = 0.8f - ((0.1f-atmo)*4.0);
					g = 1.0f;
					b = 0.5f - ((0.1f-atmo)*10.0);
				*/
				gc = GasConstituent(
					&s_sulfurDioxide, 1.0,
					vector3d(-0.1*4.0, 0.0, -0.1*10.0),
					vector3d(1.0*4.0,  0.0, 1.0*10.0)
				);
			} else {
				/*
				// N
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
				*/
				gc = GasConstituent(
					&s_nitrogen, 1.0,
					vector3d(0.0,  0.0, 0.0),
					vector3d(0.0,  0.0, 0.0)
				);
			}
		} else { // (m_volatileGas.ToDouble() < 0.01)
			gc = GasConstituent(
					&s_vacuum, 1.0,
					vector3d(0.0,  0.0, 0.0),
					vector3d(0.0,  0.0, 0.0)
			);
		}
		break;
	}
	m_gasConstituents.push_back(gc);
}


void Atmosphere::PickSingleConstituentAtmosphere(SystemBody *sbody)
{
	/* Alpha value isn't real alpha. in the shader fog depth is determined
	 * by density*alpha, so that we can have very dense atmospheres
	 * without having them a big stinking solid color obscuring everything

	  These are our atmosphere colours, for terrestrial planets we use m_atmosOxidizing
	  for some variation to atmosphere colours
	 */
	const SystemBody::BodyType type = sbody->type;
	switch (type) {
		case SystemBody::TYPE_PLANET_GAS_GIANT:
			m_atmosDensity = 14.0;
			break;
		case SystemBody::TYPE_PLANET_ASTEROID:
			m_atmosDensity = 0.0;
			break;
		default:
		case SystemBody::TYPE_PLANET_TERRESTRIAL:
			m_atmosDensity = sbody->m_volatileGas.ToDouble();
			CalculateGasConstituentsForSingleConstituentModel(sbody);
			break;
	}
}

// Calculates the colour and average atmosphere properties used in the simple scattering model
// This model assumes air is a coloured fluid made up of constituents with different colours.
// The colour is calculated by summing the weighted colours of the constituents.
// The average molar mass (M in kg per mol) and constant pressure specific heat (Cp in J per kg per mol) is similarly found by summing
// the weighted contributions. 
// The weights are based on the fraction of scatterers (particles) making up the atmosphere. 
// As Cp and M are 'per mol' of particles the weighting to obtain average values is valid.
void Atmosphere::CalculateSimpleScatteringModelProperties()
{
	Color col(0.0, 0.0, 0.0, 0.0);
	double avgMolarMass = 0.0;
	double avgConstantPressureSpecificHeat = 0.0;
	// Step through gasses and aerosols and sum the weighted colours.
	for (std::vector<GasConstituent>::iterator i = m_gasConstituents.begin(); i != m_gasConstituents.end(); i++) {
		const float fraction = float((*i).fraction);
		Color c = i->GetSimpleScatteringModelColorContrib(m_sbody);
		col.r += c.r;
		col.g += c.g;
		col.b += c.b;
		col.a += c.a;
		avgMolarMass += i->gas->M*fraction;
		avgConstantPressureSpecificHeat += i->gas->Cp*fraction;
	}
	
	for (std::vector<AerosolConstituent>::iterator i = m_aerosolConstituents.begin(); i != m_aerosolConstituents.end(); i++) {
		const float fraction = float((i)->fraction);
		Color c = i->GetSimpleScatteringModelColorContrib(m_sbody);
/*
		col.r += c.r;
		col.g += c.g;
		col.b += c.b;
		col.a += c.a;
		avgMolarMass += i->gas->M*fraction;
		acgConstantPressureSpecificHeat += i->gas->Cp*fraction;
*/
	}
	m_simpleScatteringColor = col;
	Cp = avgConstantPressureSpecificHeat; // in J/kg/mol
	M = avgMolarMass; // in kg/mol
}

bool Atmosphere::GetGasPresence(std::string name, GasConstituent *gc)
{
	gc = 0;
	bool gasFound = false;
	for (std::vector<GasConstituent>::iterator i = m_gasConstituents.begin(); i != m_gasConstituents.end(); i++) {
		if (i->gas->name == name) {
			gc = &(*i);
			gasFound = true;
			break;
		}
	}
	return gasFound;
}

bool Atmosphere::GetAerosolPresence(std::string name, AerosolConstituent *ac)
{
	ac = 0;
	bool aerosolFound = false;
	for (std::vector<AerosolConstituent>::iterator i = m_aerosolConstituents.begin(); i != m_aerosolConstituents.end(); i++) {
		if (i->aerosol->name == name) {
			ac = &(*i);
			aerosolFound = true;
			break;
		}
	}
	return aerosolFound;
}

GasConstituent::GasConstituent(const Gas *g, double fractionOfNumberOfMolecules,
					vector3d colChangeOffset_, vector3d colChangeRange_)
					: gas(g),
					 fraction(fractionOfNumberOfMolecules),
					 colChangeOffset(colChangeOffset_),
					 colChangeRange(colChangeRange_)
{
}

Color GasConstituent::GetSimpleScatteringModelColorContrib(SystemBody *sbody) const
{
	const double atmos = sbody->m_atmosOxidizing.ToDouble();
	const Color &col = gas->col;
	Color c; 
	c.r = (col.r+colChangeOffset.x+colChangeRange.x*atmos)*fraction;
	c.g = (col.g+colChangeOffset.y+colChangeRange.y*atmos)*fraction;
	c.b = (col.b+colChangeOffset.z+colChangeRange.z*atmos)*fraction;
	c.a = (col.a*fraction);
	return c;
}

void GasConstituent::PrintProperties(SystemBody *sbody) const
{
	gas->PrintProperties();
	Color c = GetSimpleScatteringModelColorContrib(sbody);
	printf("Simple scattering colour contribution: r %f, g %f, b %f, a %f\n", c.r, c.g, c.b, c.a);
	printf("Fraction of atmosphere by particle count: %f\n", fraction);
}

Gas::Gas(std::string name, 
		double constantPressureSpecificHeat, double molarMass, 
		double raylieghCoefficient, Color colour)
	: Scatterer(name, colour),
	  Cp(constantPressureSpecificHeat),
	  M(molarMass),
	  raylieghCoeff(raylieghCoefficient)
{
}

void Gas::PrintProperties() const
{
	printf("Name: %s\n", name.c_str());
	printf("Cp: %f (J/kg/mol), Molar mass %f kg/mol, Rayleigh constant %f\n",
		Cp, M, raylieghCoeff);
	const Color &c = col;
	printf("Simple scattering colour: r %f, g %f, b %f, a %f\n", c.r, c.g, c.b, c.a);
}

Color AerosolConstituent::GetSimpleScatteringModelColorContrib(SystemBody *sbody) 
{
	return aerosol->col*fraction;
}

Scatterer::Scatterer(std::string name_, Color colour)
	:name(name_),
	 col(colour)
{
}

// Calculate parameters used in the atmospheric model for shaders
// used by both LmrModels and Geosphere
void Atmosphere::CalcAtmosphereParams()
{
	AtmosphereParameters params;

	double atmosDensity = m_atmosDensity;

	params.atmosCol = m_simpleScatteringColor;
	// adjust global atmosphere opacity
	atmosDensity *= 1e-5;

	params.atmosDensity = static_cast<float>(atmosDensity);

	// Calculate parameters used in the atmospheric model for shaders
	// Isothermal atmospheric model
	// See http://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_atmospheric_pressure_variation
	// This model features an exponential decrease in pressure and density with altitude.
	// The scale height is 1/the exponential coefficient.

	// The equation for pressure is:
	// Pressure at height h = Pressure surface * e^((-Mg/RT)*h)

	// calculate (inverse) atmosphere scale height
	// The formula for scale height is:
	// h = RT / Mg
	// h is height above the surface in meters
	// R is the universal gas constant
	// T is the surface temperature in Kelvin
	// g is the gravity in m/s^2
	// M is the molar mass of air in kg/mol 

	// calculate gravity
	// radius of the planet
	const double radiusPlanet_in_m = (m_sbody->radius.ToDouble()*EARTH_RADIUS);
	const double massPlanet_in_kg = (m_sbody->mass.ToDouble()*EARTH_MASS);
	const double g = G*massPlanet_in_kg/(radiusPlanet_in_m*radiusPlanet_in_m);

	const double T = static_cast<double>(m_sbody->averageTemp);

	// Atmosphere::M is set during initialisation
	const float atmosScaleHeight = static_cast<float>(GAS_CONSTANT_R*T/(M*g));

	// min of 2.0 corresponds to a scale height of 1/20 of the planet's radius,
	params.atmosInvScaleHeight = std::max(20.0f, static_cast<float>(m_sbody->GetRadius() / atmosScaleHeight));
	// integrate atmospheric density between surface and this radius. this is 10x the scale
	// height, which should be a height at which the atmospheric density is negligible
	params.atmosRadius = 1.0f + static_cast<float>(10.0f * atmosScaleHeight) / m_sbody->GetRadius();

	params.planetRadius = static_cast<float>(radiusPlanet_in_m);

	if (!m_shaderAtmosParams) 
		m_shaderAtmosParams = new AtmosphereParameters;
	*m_shaderAtmosParams = params;
}

/*
 * dist = distance from centre
 * returns pressure in earth atmospheres
 * function is slightly different from the isothermal earth-based approximation used in shaders,
 * but it isn't visually noticeable.
 */
void Atmosphere::GetAtmosphericState(double dist, double *outPressure, double *outDensity) const
{
#if 0
	static bool atmosphereTableShown = false;
	if (!atmosphereTableShown) {
		atmosphereTableShown = true;
		for (double h = -1000; h <= 50000; h = h+1000.0) {
			double p = 0.0, d = 0.0;
			GetAtmosphericState(h+m_sbody->GetRadius(),&p,&d);
			printf("height(m): %f, pressure(kpa): %f, density: %f\n", h, p*101325.0/1000.0, d);
		}
	}
#endif

	double surfaceDensity;
	const double SPECIFIC_HEAT_AIR_CP= Cp;// constant pressure specific heat, for the combination of gasses that make up air
	const double GAS_MOLAR_MASS = M;
	const double GAS_CONSTANT = 8.3144621;
	const double PA_2_ATMOS = 1.0 / 101325.0;

	// surface gravity = -G*M/planet radius^2
	const double surfaceGravity_g = -G*m_sbody->GetMass()/pow((this->GetSystemBody()->GetRadius()),2); // should be stored in sbody
	// lapse rate http://en.wikipedia.org/wiki/Adiabatic_lapse_rate#Dry_adiabatic_lapse_rate
	// the wet adiabatic rate can be used when cloud layers are incorporated
	// fairly accurate in the troposphere
	const double lapseRate_L = -surfaceGravity_g/SPECIFIC_HEAT_AIR_CP; // negative deg/m

	const double height_h = (dist-m_sbody->GetRadius()); // height in m
	const double surfaceTemperature_T0 = m_sbody->averageTemp; //K

	surfaceDensity = m_atmosDensity;// kg / m^3
	// convert to moles/m^3
	surfaceDensity/=GAS_MOLAR_MASS;

	const double adiabaticLimit = surfaceTemperature_T0/lapseRate_L; //should be stored

	// This model has no atmosphere beyond the adiabetic limit
	if (height_h >= adiabaticLimit) {*outDensity = 0.0; *outPressure = 0.0; return;}

	//P = density*R*T=(n/V)*R*T
	const double surfaceP_p0 = PA_2_ATMOS*((surfaceDensity)*GAS_CONSTANT*surfaceTemperature_T0); // in atmospheres

	// height below zero should not occur
	if (height_h < 0.0) { *outPressure = surfaceP_p0; *outDensity = surfaceDensity*GAS_MOLAR_MASS; return; }

	//*outPressure = p0*(1-l*h/T0)^(g*M/(R*L);
	*outPressure = surfaceP_p0*pow((1-lapseRate_L*height_h/surfaceTemperature_T0),(-surfaceGravity_g*GAS_MOLAR_MASS/(GAS_CONSTANT*lapseRate_L)));// in ATM since p0 was in ATM
	//                                                                               ^^g used is abs(g)
	// temperature at height
	double temp = surfaceTemperature_T0+lapseRate_L*height_h;

	*outDensity = (*outPressure/(PA_2_ATMOS*GAS_CONSTANT*temp))*GAS_MOLAR_MASS;
}