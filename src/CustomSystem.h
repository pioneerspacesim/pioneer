#ifndef _CUSTOMSYSTEM_H
#define _CUSTOMSYSTEM_H

#include "StarSystem.h"
#include "Polit.h"
#include "vector3.h"
#include "fixed.h"

#include "oolua/oolua.h"
#include "PiLuaClasses.h"

class CustomSBody {
public:
	CustomSBody() {}

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

	// lua interface
	CustomSBody(std::string s, int type);

	inline void l_radius(pi_fixed& r) { radius = r; }
	inline void l_mass(pi_fixed& m) { mass = m; }
	inline void l_temp(int t) { averageTemp = t; }
	inline void l_semi_major_axis(pi_fixed& n) { semiMajorAxis = n; }
	inline void l_eccentricity(pi_fixed& e) { eccentricity = e; }
	inline void l_latitude(float l) { latitude = l; }
	inline void l_longitude(float l) { longitude = l; }
	inline void l_rotation_period(pi_fixed &p) { rotationPeriod = p; }
	inline void l_axial_tilt(pi_fixed &t) { axialTilt = t; }
	inline void l_height_map(std::string f) { heightMapFilename = f; }

	inline void l_add(CustomSBody& sbody) { children.push_back(sbody); }
};

OOLUA_CLASS_NO_BASES(CustomSBody)
	OOLUA_TYPEDEFS
		No_default_constructor
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_2(std::string, int)
	OOLUA_CONSTRUCTORS_END

	OOLUA_MEM_FUNC_1_RENAME(radius, void, l_radius, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(mass, void, l_mass, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(temp, void, l_temp, int)
	OOLUA_MEM_FUNC_1_RENAME(semi_major_axis, void, l_semi_major_axis, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(eccentricity, void, l_eccentricity, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(latitude, void, l_latitude, float)
	OOLUA_MEM_FUNC_1_RENAME(inclination, void, l_latitude, float)  // duplicate, latitude has different meaning for orbiting things
	OOLUA_MEM_FUNC_1_RENAME(longitude, void, l_longitude, float)
	OOLUA_MEM_FUNC_1_RENAME(rotation_period, void, l_rotation_period, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(axial_tilt, void, l_axial_tilt, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(height_map, void, l_height_map, std::string)
	OOLUA_MEM_FUNC_1_RENAME(add, void, l_add, CustomSBody&)
OOLUA_CLASS_END

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

	// lua interface
	CustomSystem(std::string s, OOLUA::Lua_table t);

	inline void l_seed(int s) { seed = s; }
	inline void l_govtype(int t) { govType = static_cast<Polit::GovType>(t); }
	inline void l_short_desc(std::string s) { shortDesc = s; }
	inline void l_long_desc(std::string s) { longDesc = s; }

	inline void l_primary_star(CustomSBody& star) { sBody = star; }
    
	void l_add_to_sector(int x, int y, pi_vector& v);
};

OOLUA_CLASS_NO_BASES(CustomSystem)
	OOLUA_TYPEDEFS
		No_default_constructor
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_2(std::string, OOLUA::Lua_table)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_1_RENAME(seed, void, l_seed, int)
	OOLUA_MEM_FUNC_1_RENAME(govtype, void, l_govtype, int)
	OOLUA_MEM_FUNC_1_RENAME(short_desc, void, l_short_desc, std::string)
	OOLUA_MEM_FUNC_1_RENAME(long_desc, void, l_long_desc, std::string)
	OOLUA_MEM_FUNC_1_RENAME(primary_star, void, l_primary_star, CustomSBody&)
	OOLUA_MEM_FUNC_3_RENAME(add_to_sector, void, l_add_to_sector, int, int, pi_vector&)
OOLUA_CLASS_END

#endif /* _CUSTOMSYSTEM_H */
