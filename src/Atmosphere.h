// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _ATMOSPHERE_H
#define _ATMOSPHERE_H

#include "Libs.h"
#include "vector3.h"
#include "Color.h"
#include "galaxy/StarSystem.h"

/*
* These classes contain descriptions of atmospheres relevant to:
*     1. Gameplay:  scooping for fuel /oxygen, habitation worlds, 
*                   faction expansion/worth of worlds, protection 
*                   from sight/certain scanning frequencies by aerosol
*     2. Rendering: atmosphere colour, sunsets/sunrises, 
*                   aerosol layers, distribution of scatterers
*     3. Physics:   variation in density/drag/buoyancy, pressure, temperature.
*
*
* SystemBody, Terrain, and sysgen concerns:
*                 1. Pioneer requires data to be generated about systems the player is not in.
*                    Trade/pirate/military/economy AI, sector view screen, faction behaviour, mission generation etc.
*                        1.1 Different levels of details for different purposes.
*                 2. Sysgen uses number sequences with very long state (seeds) internally. 
*                    Common int32 body seed only allows for 4bn possibilities. 
*                    There are much greater than 4bn stars in the galaxy alone.
*                       2.1 Preferable to use the properly seeded and large number sequences in the main sysgen
*                           to generate some of the important parameters e.g. atmosOxidizing.
*                 3. The best option seems to be to do generation in stages. In the eventuality sysgen 
*                    needs some detailed atmosphere data, it's possible to generate it in 
*                    reverse - the gameplay consequences first and derive properties from that.
*                 4. Atmospheres should depend on terrain classes. Part of the terrain property gen now in Terrain:: could be moved to another class.
*                 5. One option is a second AtmosphereBody class which 
*                    houses the later stages of physics/rendering and generates verticies etc.
*
* Class boundaries/division philosophy: 
* (As things currently stand. This will change as functionality requirements with advanced scattering etc. 
  become clear and this will be updated.)
* Succinctly: Atmosphere:
*                 1. Atmospheres contain lists of constituents. 
*                        1.1 The constituents can be aersols or gasses.
*                 2. Atmosphere class is responsible for:
*                        2.1 Creating descriptions of the atmosphere including 
*                            the constituents.
*                        2.2 Calculating all the quantities needed for gameplay, physics 
*                            and rendering from the constituents.
*                                2.21 e.g. Scale height(s), Overall particle density, 
*                                     average scattering coefficient etc.
*                        2.3 Interfaces for relevant functions
*                                2.31 e.g. to check if a particular element or gas is present 
*                                or calculate concentration.
*             Constituents:
*                  3. Constituents contain a description of the role an aerosol or gas 
*                     plays in the atmosphere. 
*                          3.1 e.g. the fraction of the atmosphere composed
*                               of a constituent or the scale height of an aerosol
*                  4. Constituent classes contain links to the constant classes 
*                     describing Gas/Aerosol properties.
*             Gas and Aerosol static const objects:
*                  5. Contain relevant properties, and chemical composition as lists of substances
*                     if applicable.
*             Substance static const objects:
*                 6. Contains descriptions of elements (e.g. H or O) and substances with a
*                    chemical formula (e.g. water H2O). Substances 
*                 7. The idea is to make adding new substances with different chemical 
*                    compositions easy.
*                 8. Will also be used for other things like planet resources/minerals/mining/ eventually.
*
* Background Info (Eventually:).. this should contain enough information for a new contributior to make a start 
* and improve things. Also someone unfamiliar to should be able to reasonably quickly check specific functionality/confirm something 
* without having to learn everything.) 
* (It just contains stuff related to related to rendering)
* Atmospheres contain scatterers (particles). These can be Aerosols or Gas molecules.
* Scatteres can bounce (and also absorb) light particles affecting what is seen.
* Scattering depends on the particle density rather than mass density. 
* i.e. the same number of light particles with similar scattering characteristics
* in a volume will affect light as much as the same number of lighter particles with similar scattering properties.
* **This means we need to deal with the particle density rather than the fractions of the mass
* In point form:
* Aerosols: Large particles (>1 micrometer) compared visible light wavelengths  
*           (=400-700 nanometers).'Mie scattering' in scattering models/shaders.
*           Can absorb light. e.g. Dust and other particulate matter 
*           (e.g. from volcanism) as well as man made pollutants 
*           smog, smoke, particles from massive explosions etc. Aerosols are important 
*           in that they make otherwise colourless atmospheres interesting (e.g. Mars)
*           Each type can have different distribution (different scale height).
*           
*
* Gasses: Molecules much smaller than the wave lengths of visible light. 
*         Subject to 'Rayliegh scattering' in models/shaders.
*         Made up of multiple elements with a chemical formula. e.g. CH4 ==> methane.
*         Are usually well mixed to a great degree - i.e. retain constant proportions (same scale height gasses).
*         in most scattering models. Do not absorb light. Note: Trace amounts of exotic gasses/aerosols
*         give gas giants a lot of colour.

* ElementComposition:  Properties needed to derive game relevant quantities such as the concentration of a 
*                      particular element in a gas, for scooping.
* ChemicalComposition: Chemical composition of a substance i.e. it's chemical formula. Relevant properties of that substance.
*
*/

/*
* To test - Call Atmosphere::SystemBody::TestSingleConstituentModelAgainstOldVersion() in Sysgen just before
            before SystemBody::InitAtmosphere is called in PickPlanetType in starsystems.cpp. This test can be removed eventually.
*/
class Scatterer {
public:
	Scatterer(std::string name, Color colour);
	std::string name;
	// colour used for simple scattering model
	Color col;
};

class Aerosol : public Scatterer {
	Aerosol(std::string name, Color colour) : Scatterer (name, colour) { };
	// parameters in calculating aerosol layers or scale heights
};

class AerosolConstituent {
public:
	AerosolConstituent() { };
	Color GetSimpleScatteringModelColorContrib(SystemBody *sbody);
	const Aerosol *aerosol;
	double fraction;
	// seperate scale heights for aerosol
	double scaleHeight;
};

class Gas : public Scatterer {
public:
	Gas(std::string name, double constantPressureSpecificHeat, 
		double molarMass, double raylieghCoefficient, Color colour);
	void PrintProperties() const;

	// MolecularComposition composition;
	// Constant pressure specific heat

	double Cp;
	// Molar mass of gas in kg/mol 
	// (mass per 1 mole of gas particles) 
	double M;
	double raylieghCoeff;
};

// the fraction of molecules, distribution, etc. of a particular gas 
// in the atmosphere
// includes
class GasConstituent {
public:
	GasConstituent() { }
	GasConstituent(const Gas *g, double fractionOfNumberOfMolecules,
		vector3d colChangeOffset_, vector3d colChangeRange_);

	Color GetSimpleScatteringModelColorContrib(SystemBody *sbody) const;
	void PrintProperties(SystemBody *sbody) const;
	
	const Gas *gas;
	// fraction of the atmosphere by number of particles
	double fraction;

	// Colour equation used in the old one constituent based colour atmosphere
	// colour.r = 
	//	gas->col.r + colChangeOffset.x + colChangeRange.y*atmosOxidising
	// similarly for g and b c
	vector3d colChangeOffset;
	vector3d colChangeRange;

	// whether the 
	// bool wellMixed;
	// which of the vailable scale heights to use
	// double scaleHeightIdx
};


class Atmosphere {
public:
	Atmosphere() { };
	~Atmosphere();
	Atmosphere(SystemBody *sbody);
	void Init(SystemBody *sbody);
	// Single Gas model (old model)
	void Atmosphere::CalculateGasConstituentsForSingleConstituentModel(SystemBody *sbody);
	void Atmosphere::PickSingleConstituentAtmosphere(SystemBody *sbody);
	// Multiple gas and aerosol model
	void Atmosphere::CalculateGasConstituents(SystemBody *sbody) { };
	void Atmosphere::CalculateAerosolConstituents(SystemBody *sbody) { };
	void CalculateSimpleScatteringModelColor();

	double GetSurfaceDensity() const { return m_atmosDensity; }
	Color GetSimpleScatteringColor() const { return m_simpleScatteringColor; }
	SystemBody *GetSystemBody() const { return m_sbody; }
	std::vector<GasConstituent> GetGasConstituents() const { return m_gasConstituents; }
	bool GetGasPresence(std::string name, GasConstituent *gc);
	std::vector<AerosolConstituent> GetAerosolConstituents() const { return m_aerosolConstituents; }
	bool GetAerosolPresence(std::string name, AerosolConstituent *ac);

	// Physics
	void Atmosphere::GetAtmosphericState(double dist, double *outPressure, double *outDensity) const;

	// Rendering
	// Used to store data relevant to shaders
	struct AtmosphereParameters {
		float atmosRadius;
		float atmosInvScaleHeight;
		float atmosDensity;
		float planetRadius;
		Color atmosCol;
		vector3d center;
		float scale;
		AtmosphereParameters() : atmosRadius(0.0), atmosInvScaleHeight(0.0),
			atmosDensity(0.0), planetRadius(0.0), atmosCol(Color(0.0)),
			center(vector3d(0.0)), scale(0.0)
		{
		}
	};
	// Requires CalcAtmospheres to be run first. SystemBody provides an interface
	AtmosphereParameters *GetAtmosphereParams() const { return m_shaderAtmosParams; }
	// Not run during sysgen to save computation when complete generation is not needed
	void CalcAtmosphereParams();

private:
	SystemBody *m_sbody;
	
	std::vector<GasConstituent> m_gasConstituents;
	std::vector<AerosolConstituent> m_aerosolConstituents;

	Color m_simpleScatteringColor;
	
	// Molar mass of well mixed atmosphere gasses
	// (mass of 1 mole of average particles)
	double M;              // kg/mol
	// Specific Heat of well mixed atmosphere gasses
	// The energy in Joules required to raise one kg 
	// of atmosphere by 1 degree Kelvin
	// This changes a bit for each type of gas molecule as 
	// the temperature changes
	double Cp;             // J/kg/K
	double m_atmosDensity; // kg/m^3
	double m_atmosOxidizing;

	AtmosphereParameters *m_shaderAtmosParams;

public:
	// Gasses
	static const Gas s_argon;              // Ar
	static const Gas s_carbonDioxide;      // CO2
	static const Gas s_carbonMonoxide;     // CO
	static const Gas s_helium;             // He
	static const Gas s_hydrogen;           // H2
	static const Gas s_oxygen;		       // O2
	static const Gas s_methane;            // CH4
	static const Gas s_nitrogen;           // N2
	static const Gas s_sulfurDioxide;     // SO2
	static const Gas s_vacuum;             // vacuum
	static const Gas s_whiteGas;           // whiteGas
	static const Gas s_earthAtmosphere;    // Earth atmosphere properties
	static const Gas s_gasGiantAtmosphere; // Single constituent atmosphere

	// Aerosols - scatterers with radius on the same scale or larger
	//            than visible wavelengths (400-800 nanometers). 
	//            Wavelengths ~1 micrometer or larger.
	//            
	//static const AerosolConstituent MartianDust;
};

// Not tested

/*
* Chemical composition - used to derive properties as well as for gameplay 
*                        reasons (scooping for certain elements like oxygen, 
*                        hydrogen, corrosive effects)
*/
/*

struct ElementComposition {
public:
	ElementComposition() : atomicNumber(0), numAtoms(0) { };
	ElementComposition(int atomicNum, int numAtomsInMolecule) {
		atomicNumber = atomicNum;
		numAtoms = numAtomsInMolecule;
	}

	// number of protons 
	int atomicNumber;
	// number of atoms in the molecule
	int numAtoms;
	// average mass of the atom in atomic weights. symbol Ar
	// this depends on the average isotope distribution, binding energy etc.
	// double atomicMass; 
};

class MolecularComposition {
public:
	MolecularComposition() : formula(0) { };
	MolecularComposition(std::string chemicalFormula, std::vector<ElementComposition> ElementCompositions): 
		formula(chemicalFormula),
		compositions(ElementCompositions)
	{
	};

	std::string formula;
	std::vector<ElementComposition> compositions;
};
*/

#endif /* _ATMOSPHERE_H */
