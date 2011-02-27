#ifndef _CUSTOMSYSTEH
#define _CUSTOMSYSTEH

#include "StarSystem.h"
#include "Polit.h"
#include "vector3.h"
#include "fixed.h"

class CustomSBody {
public:
	std::string            name;
	SBody::BodyType        type;
	fixed                  radius; // in earth radii for planets, sol radii for stars
	fixed                  mass; // earth masses or sol masses
	int                    averageTemp; // kelvin
	fixed                  semiMajorAxis; // in AUs
	fixed                  eccentricity;
	// for orbiting things, latitude = inclination
	float                  latitude, longitude; // radians
	fixed                  rotationPeriod; // in days
	fixed                  axialTilt; // in radians
	std::string            heightMapFilename;
	std::list<CustomSBody> children;

	/* composition */
	fixed metallicity; // (crust) 0.0 = light (Al, SiO2, etc), 1.0 = heavy (Fe, heavy metals)
	fixed volatileGas; // 1.0 = earth atmosphere density
	fixed volatileLiquid; // 1.0 = 100% ocean cover (earth = 70%)
	fixed volatileIces; // 1.0 = 100% ice cover (earth = 3%)
	fixed volcanicity; // 0 = none, 1.0 = fucking volcanic
	fixed atmosOxidizing; // 0.0 = reducing (H2, NH3, etc), 1.0 = oxidising (CO2, O2, etc)
	fixed life; // 0.0 = dead, 1.0 = teeming
};

class CustomSystem {
public:
	static void Init();
	static const std::list<const CustomSystem*> GetCustomSystemsForSector(int sectorX, int sectorY);
	static const CustomSystem* GetCustomSystem(const char* name);
	static const SBodyPath GetSBodyPathForCustomSystem(const CustomSystem* cs);
	static const SBodyPath GetSBodyPathForCustomSystem(const char* name);

	std::string            name;
    CustomSBody            sBody;
	SBody::BodyType        primaryType[4];
	int                    sectorX, sectorY;
	vector3f               pos;
	Uint32                 seed;
	Polit::GovType         govType;
	std::string            shortDesc;
	std::string            longDesc;

	bool IsRandom() const { return sBody.name.length() == 0; }
};

#endif /* _CUSTOMSYSTEH */
