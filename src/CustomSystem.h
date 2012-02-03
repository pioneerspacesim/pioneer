#ifndef _CUSTOMSYSTEM_H
#define _CUSTOMSYSTEM_H

#include "StarSystem.h"
#include "Polit.h"
#include "vector3.h"
#include "fixed.h"

#include "oolua/oolua.h"
#include "PiLuaClasses.h"
#include "LuaConstants.h"

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
	fixed                  orbitalOffset;
	bool                   want_rand_offset;
	// for orbiting things, latitude = inclination
	float                  latitude, longitude; // radians
	fixed                  rotationPeriod; // in days
	fixed                  axialTilt; // in radians
	std::string            heightMapFilename;
	int                    heightMapFractal;
	std::list<CustomSBody> children;

	/* composition */
	fixed metallicity; // (crust) 0.0 = light (Al, SiO2, etc), 1.0 = heavy (Fe, heavy metals)
	fixed volatileGas; // 1.0 = earth atmosphere density
	fixed volatileLiquid; // 1.0 = 100% ocean cover (earth = 70%)
	fixed volatileIces; // 1.0 = 100% ice cover (earth = 3%)
	fixed volcanicity; // 0 = none, 1.0 = fucking volcanic
	fixed atmosOxidizing; // 0.0 = reducing (H2, NH3, etc), 1.0 = oxidising (CO2, O2, etc)
	fixed life; // 0.0 = dead, 1.0 = teeming

	Uint32 seed;
	bool   want_rand_seed;

	// lua interface
	CustomSBody(std::string s, std::string stype);

	inline CustomSBody* l_radius(pi_fixed& r) { radius = r; return this; }
	inline CustomSBody* l_mass(pi_fixed& m) { mass = m; return this; }
	inline CustomSBody* l_temp(int t) { averageTemp = t; return this; }
	inline CustomSBody* l_semi_major_axis(pi_fixed& n) { semiMajorAxis = n; return this; }
	inline CustomSBody* l_eccentricity(pi_fixed& e) { eccentricity = e; return this; }
	inline CustomSBody* l_orbital_offset(pi_fixed& o) { orbitalOffset = o; want_rand_offset = false; return this; }
	inline CustomSBody* l_latitude(float l) { latitude = l; return this; }
	inline CustomSBody* l_longitude(float l) { longitude = l; return this; }
	inline CustomSBody* l_rotation_period(pi_fixed &p) { rotationPeriod = p; return this; }
	inline CustomSBody* l_axial_tilt(pi_fixed &t) { axialTilt = t; return this; }

	CustomSBody *l_height_map(lua_State *L, std::string f, unsigned int n);

	inline CustomSBody* l_metallicity(pi_fixed& f) { metallicity = f; return this; }
	inline CustomSBody* l_volcanicity(pi_fixed& f) { volcanicity = f; return this; }
	inline CustomSBody* l_atmos_density(pi_fixed& f) { volatileGas = f; return this; }
	inline CustomSBody* l_atmos_oxidizing(pi_fixed& f) { atmosOxidizing = f; return this; }
	inline CustomSBody* l_ocean_cover(pi_fixed& f) { volatileLiquid = f; return this; }
	inline CustomSBody* l_ice_cover(pi_fixed& f) { volatileIces = f; return this; }
	inline CustomSBody* l_life(pi_fixed& f) { life = f; return this; }

	inline CustomSBody* l_seed(int s) { seed = s; want_rand_seed = false; return this; }
};

OOLUA_CLASS_NO_BASES(CustomSBody)
	OOLUA_TYPEDEFS
		No_default_constructor
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_2(std::string, std::string)
	OOLUA_CONSTRUCTORS_END

	OOLUA_MEM_FUNC_1_RENAME(seed, CustomSBody*, l_seed, int)
	OOLUA_MEM_FUNC_1_RENAME(radius, CustomSBody*, l_radius, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(mass, CustomSBody*, l_mass, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(temp, CustomSBody*, l_temp, int)
	OOLUA_MEM_FUNC_1_RENAME(semi_major_axis, CustomSBody*, l_semi_major_axis, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(eccentricity, CustomSBody*, l_eccentricity, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(orbital_offset, CustomSBody*, l_orbital_offset, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(latitude, CustomSBody*, l_latitude, float)
	OOLUA_MEM_FUNC_1_RENAME(inclination, CustomSBody*, l_latitude, float)  // duplicate, latitude has different meaning for orbiting things
	OOLUA_MEM_FUNC_1_RENAME(longitude, CustomSBody*, l_longitude, float)
	OOLUA_MEM_FUNC_1_RENAME(rotation_period, CustomSBody*, l_rotation_period, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(axial_tilt, CustomSBody*, l_axial_tilt, pi_fixed&)
	OOLUA_MEM_FUNC_3_RENAME(height_map, CustomSBody*, l_height_map, lua_State*, std::string, unsigned int)
	OOLUA_MEM_FUNC_1_RENAME(metallicity, CustomSBody*, l_metallicity, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(volcanicity, CustomSBody*, l_volcanicity, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(atmos_density, CustomSBody*, l_atmos_density, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(atmos_oxidizing, CustomSBody*, l_atmos_oxidizing, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(ocean_cover, CustomSBody*, l_ocean_cover, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(ice_cover, CustomSBody*, l_ice_cover, pi_fixed&)
	OOLUA_MEM_FUNC_1_RENAME(life, CustomSBody*, l_life, pi_fixed&)
OOLUA_CLASS_END

class CustomSystem {
public:
	static void Init();
	static const std::list<const CustomSystem*> GetCustomSystemsForSector(int sectorX, int sectorY, int sectorZ);
	static const CustomSystem* GetCustomSystem(const char* name);
	static const SystemPath GetPathForCustomSystem(const CustomSystem* cs);
	static const SystemPath GetPathForCustomSystem(const char* name);

	std::string            name;
    CustomSBody            sBody;
	SBody::BodyType        primaryType[4];
	int                    numStars;
	int                    sectorX, sectorY, sectorZ;
	vector3f               pos;
	Uint32                 seed;
	Polit::GovType         govType;
	std::string            shortDesc;
	std::string            longDesc;

	bool IsRandom() const { return sBody.name.length() == 0; }

	// lua interface
	CustomSystem(std::string s, OOLUA::Lua_table t);

	inline CustomSystem* l_seed(int s) { seed = s; return this; }
	inline CustomSystem* l_short_desc(std::string s) { shortDesc = s; return this; }
	inline CustomSystem* l_long_desc(std::string s) { longDesc = s; return this; }

	CustomSystem* l_govtype(std::string st);

	void l_bodies(lua_State* L, CustomSBody& primary_star, OOLUA::Lua_table t);

	void l_add_to_sector(int x, int y, int z, pi_vector& v);
};

OOLUA_CLASS_NO_BASES(CustomSystem)
	OOLUA_TYPEDEFS
		No_default_constructor
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_2(std::string, OOLUA::Lua_table)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_1_RENAME(seed, CustomSystem*, l_seed, int)
	OOLUA_MEM_FUNC_1_RENAME(govtype, CustomSystem*, l_govtype, std::string)
	OOLUA_MEM_FUNC_1_RENAME(short_desc, CustomSystem*, l_short_desc, std::string)
	OOLUA_MEM_FUNC_1_RENAME(long_desc, CustomSystem*, l_long_desc, std::string)
	OOLUA_MEM_FUNC_3_RENAME(bodies, void, l_bodies, lua_State*, CustomSBody&, OOLUA::Lua_table)
	OOLUA_MEM_FUNC_4_RENAME(add_to_sector, void, l_add_to_sector, int, int, int, pi_vector&)
OOLUA_CLASS_END

#endif /* _CUSTOMSYSTEM_H */
