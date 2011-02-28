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

	// lua interface
	inline CustomSystem(std::string s) { name = s; }
	inline void l_sector(int x, int y) { sectorX = x; sectorY = y; }
	inline void l_pos(pi_vector& v) { pos = v; }
	inline void l_seed(int s) { seed = s; }
	inline void l_govtype(int t) { govType = static_cast<Polit::GovType>(t); }
	inline void l_short_desc(std::string s) { shortDesc = s; }
	inline void l_long_desc(std::string s) { longDesc = s; }

	void l_type(OOLUA::Lua_table t);
	void l_add_to_universe();
};

OOLUA_CLASS_NO_BASES(CustomSystem)
	OOLUA_TYPEDEFS
		No_default_constructor
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_1(std::string)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_1_RENAME(type, void, l_type, OOLUA::Lua_table)
	OOLUA_MEM_FUNC_2_RENAME(sector, void, l_sector, int, int)
	OOLUA_MEM_FUNC_1_RENAME(pos, void, l_pos, pi_vector&)
	OOLUA_MEM_FUNC_1_RENAME(seed, void, l_seed, int)
	OOLUA_MEM_FUNC_1_RENAME(govtype, void, l_govtype, int)
	OOLUA_MEM_FUNC_1_RENAME(short_desc, void, l_short_desc, std::string)
	OOLUA_MEM_FUNC_1_RENAME(long_desc, void, l_long_desc, std::string)
	OOLUA_MEM_FUNC_0_RENAME(add_to_universe, void, l_add_to_universe)
OOLUA_CLASS_END

#endif /* _CUSTOMSYSTEM_H */
