#ifndef _CUSTOM_STARSYSTEMS_H
#define _CUSTOM_STARSYSTEMS_H

#include "StarSystem.h"
#include "fixed.h"
#include "Polit.h"

struct CustomSBody {
	const char *name; // null to end system
	SBody::BodyType type;
	int primaryIdx;  // -1 for primary
	fixed radius; // in earth radii for planets, sol radii for stars
	fixed mass; // earth masses or sol masses
	int averageTemp; // kelvin
	fixed semiMajorAxis; // in AUs
	fixed eccentricity;
	// for orbiting things, latitude = inclination
	struct {
		float latitude, longitude; // radians
	};
	fixed rotationPeriod; // in days
	fixed axialTilt; // in radians
	struct {
		fixed metallicity; // (crust) 0.0 = light (Al, SiO2, etc), 1.0 = heavy (Fe, heavy metals)
		fixed volatileGas; // 1.0 = earth atmosphere density
		fixed volatileLiquid; // 1.0 = 100% ocean cover (earth = 70%)
		fixed volatileIces; // 1.0 = 100% ice cover (earth = 3%)
		fixed volcanicity; // 0 = none, 1.0 = fucking volcanic
		fixed atmosOxidizing; // 0.0 = reducing (H2, NH3, etc), 1.0 = oxidising (CO2, O2, etc)
		fixed life; // 0.0 = dead, 1.0 = teeming
	} composition;
	int econType; // StarSystem.cpp enum ECON_XXX
	const char *heightMapFilename;
};

struct CustomSystem {
	const char *name;
	const CustomSBody *sbodies; // 0 to let system be random
	SBody::BodyType primaryType[4];
	int sectorX, sectorY;
	vector3f pos;
	Uint32 seed;
	const char *shortDesc;
	const char *longDesc;
	Polit::GovType govType;
};

extern const CustomSystem custom_systems[];

#endif /* _CUSTOM_STARSYSTEMS_H */
