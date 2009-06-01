#include "StarSystem.h"
#include "Sector.h"
#include "custom_starsystems.h"
#include "Serializer.h"
#include "NameGenerator.h"
#include "GeoSphere.h"

#define CELSIUS	273.15
#define DEBUG_DUMP

// minimum moon mass a little under Europa's
static const fixed MIN_MOON_MASS = fixed(1,30000); // earth masses
static const fixed MIN_MOON_DIST = fixed(15,10000); // AUs
static const fixed MAX_MOON_DIST = fixed(2, 100); // AUs
// if binary stars have separation s, planets can have stable
// orbits at (0.5 * s * SAFE_DIST_FROM_BINARY)
static const fixed SAFE_DIST_FROM_BINARY = fixed(5,1);
static const fixed PLANET_MIN_SEPARATION = fixed(135,100);

// very crudely
static const fixed AU_SOL_RADIUS = fixed(305,65536);
static const fixed AU_EARTH_RADIUS = fixed(3, 65536);

// indexed by enum type turd  
float StarSystem::starColors[][3] = {
	{ 0, 0, 0 }, // gravpoint
	{ 0.5, 0.0, 0.0 }, // brown dwarf
	{ 1.0, 0.2, 0.0 }, // M
	{ 1.0, 0.6, 0.1 }, // K
	{ 0.4, 0.4, 0.8 }, // white dwarf
	{ 1.0, 1.0, 0.4 }, // G
	{ 1.0, 1.0, 0.8 }, // F
	{ 1.0, 1.0, 1.0 }, // A
	{ 0.7, 0.7, 1.0 }, // B
	{ 1.0, 0.7, 1.0 }  // O
};

// indexed by enum type turd  
float StarSystem::starRealColors[][3] = {
	{ 0, 0, 0 }, // gravpoint
	{ 0.5, 0.0, 0.0 }, // brown dwarf
	{ 1.0, 0.2, 0.0 }, // M
	{ 1.0, 0.7, 0.1 }, // K
	{ 1.0, 1.0, 1.0 }, // white dwarf
	{ 1.0, 1.0, 0.9 }, // G
	{ 1.0, 1.0, 1.0 }, // F
	{ 1.0, 1.0, 1.0 }, // A
	{ 0.7, 0.7, 1.0 }, // B
	{ 1.0, 0.7, 1.0 }  // O
};

static const struct SBodySubTypeInfo {
	SBody::BodySuperType supertype;
	int mass[2]; // min,max % sol for stars, unused for planets
	int radius; // % sol radii for stars, % earth radii for planets
	const char *description;
	const char *icon;
	int tempMin, tempMax;
} bodyTypeInfo[SBody::TYPE_MAX] = {
	{
		SBody::SUPERTYPE_NONE, {}, 0, "Shouldn't see this!",
	}, {
		SBody::SUPERTYPE_STAR,
		{2,8}, 30, "Brown dwarf sub-stellar object",
		"icons/object_brown_dwarf.png",
		1000, 2000
	}, {
		SBody::SUPERTYPE_STAR,
		{10,47}, 50, "Type 'M' red star",
		"icons/object_star_m.png",
		2000, 3500
	}, {
		SBody::SUPERTYPE_STAR,
		{50,78}, 90, "Type 'K' orange star",
		"icons/object_star_k.png",
		3500, 5000
	}, {
		SBody::SUPERTYPE_STAR,
		{20,100}, 1, "White dwarf",
		"icons/object_white_dwarf.png",
		4000, 40000
	}, { 
		SBody::SUPERTYPE_STAR,
		{80,110}, 110, "Type 'G' yellow star",
		"icons/object_star_g.png",
		5000, 6000
	}, {
		SBody::SUPERTYPE_STAR,
		{115,170}, 140, "Type 'F' white star",
		"icons/object_star_f.png",
		6000, 7500
	}, {
		SBody::SUPERTYPE_STAR,
		{180,320}, 210, "Type 'A' hot white star",
		"icons/object_star_a.png",
		7500, 10000
	}, {
		SBody::SUPERTYPE_STAR,
		{400,1800}, 700, "Bright type 'B' blue star",
		"icons/object_star_b.png",
		10000, 30000
	}, {
		SBody::SUPERTYPE_STAR,
		{2000,4000}, 1600, "Hot, massive type 'O' blue star",
		"icons/object_star_o.png",
		30000, 60000
	}, {
		SBody::SUPERTYPE_GAS_GIANT,
		{}, 390, "Small gas giant",
		"icons/object_planet_small_gas_giant.png"
	}, {
		SBody::SUPERTYPE_GAS_GIANT,
		{}, 950, "Medium gas giant",
		"icons/object_planet_medium_gas_giant.png"
	}, {
		SBody::SUPERTYPE_GAS_GIANT,
		{}, 1110, "Large gas giant",
		"icons/object_planet_large_gas_giant.png"
	}, {
		SBody::SUPERTYPE_GAS_GIANT,
		{}, 1500, "Very large gas giant",
		"icons/object_planet_large_gas_giant.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 1, "Asteroid",
		"icons/object_planet_asteroid.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 2, "Large asteroid",
		"icons/object_planet_asteroid.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 26, "Small, rocky dwarf planet", // moon radius
		"icons/object_planet_dwarf.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 52, "Small, rocky planet with a thin atmosphere", // mars radius
		"icons/object_planet_small.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with liquid water and a nitrogen atmosphere", // earth radius
		"icons/object_planet_water_n2.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a carbon dioxide atmosphere",
		"icons/object_planet_co2.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a methane atmosphere",
		"icons/object_planet_methane.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with liquid water and a thick nitrogen atmosphere",
		"icons/object_planet_water_n2.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a thick carbon dioxide atmosphere",
		"icons/object_planet_co2.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a thick methane atmosphere",
		"icons/object_planet_methane.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Highly volcanic world",
		"icons/object_planet_volcanic.png"
	}, {
		SBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "World with indigenous life and an oxygen atmosphere",
		"icons/object_planet_life.png"
	}, {
		SBody::SUPERTYPE_STARPORT,
		{}, 0, "Orbital starport",
		"icons/object_orbital_starport.png"
	}, {
		SBody::SUPERTYPE_STARPORT,
		{}, 0, "Starport",
	}
};

SBody::BodySuperType SBody::GetSuperType() const
{
	return bodyTypeInfo[type].supertype;
}

const char *SBody::GetAstroDescription()
{
	return bodyTypeInfo[type].description;
}

const char *SBody::GetIcon()
{
	return bodyTypeInfo[type].icon;
}

/*
 * Position a surface starport on dry land!
 */
static void position_settlement_on_planet(SBody *b)
{
	MTRand r(b->seed);
	GeoSphere geo(b->parent);
	double height;
	int tries;
	for (tries=0; tries<100; tries++) {
		// used for orientation on planet surface
		b->orbit.rotMatrix = matrix4x4d::RotateZMatrix(2*M_PI*r.Double()) *
				      matrix4x4d::RotateYMatrix(2*M_PI*r.Double());
		vector3d pos = b->orbit.rotMatrix * vector3d(0,1,0);
		pos = pos.Normalized();
		height = geo.GetHeight(pos);
		// don't want to be under water
		if (height != 0) break;
	}
	//printf("%d height %.20f\n", tries, height);
}

double SBody::GetMaxChildOrbitalDistance() const
{
	double max = 0;
	for (unsigned int i=0; i<children.size(); i++) {
		if (children[i]->orbMax.ToDouble() > max) {
			max = children[i]->orbMax.ToDouble();	
		}
	}
	return AU * max;
}


static inline Sint64 isqrt(Sint64 a)
{
	Sint64 ret=0;
	Sint64 s;
	Sint64 ret_sq=-a-1;
	for(s=62; s>=0; s-=2){
		Sint64 b;
		ret+= ret;
		b=ret_sq + ((2*ret+1)<<s);
		if(b<0){
			ret_sq=b;
			ret++;
		}
	}
	return ret;
}


/*
 * These are the nice floating point surface temp calculating turds.
 *
static const double boltzman_const = 5.6704e-8;
static double calcEnergyPerUnitAreaAtDist(double star_radius, double star_temp, double object_dist)
{
	const double total_solar_emission = boltzman_const *
		star_temp*star_temp*star_temp*star_temp*
		4*M_PI*star_radius*star_radius;

	return total_solar_emission / (4*M_PI*object_dist*object_dist);
}

// bond albedo, not geometric
static double CalcSurfaceTemp(double star_radius, double star_temp, double object_dist, double albedo, double greenhouse)
{
	const double energy_per_meter2 = calcEnergyPerUnitAreaAtDist(star_radius, star_temp, object_dist);
	const double surface_temp = pow(energy_per_meter2*(1-albedo)/(4*(1-greenhouse)*boltzman_const), 0.25);
	return surface_temp;
}
*/
/*
 * Instead we use these butt-ugly overflow-prone spat of ejaculate:
 */
/*
 * star_radius in sol radii
 * star_temp in kelvin,
 * object_dist in AU
 * return Watts/m^2
 */
static fixed calcEnergyPerUnitAreaAtDist(fixed star_radius, int star_temp, fixed object_dist)
{
	fixed temp = star_temp * fixed(1,10000);
	const fixed total_solar_emission =
		temp*temp*temp*temp*star_radius*star_radius;
	
	return fixed(1744665451,100000)*(total_solar_emission / (object_dist*object_dist));
}

static int CalcSurfaceTemp(const SBody *primary, fixed distToPrimary, fixed albedo, fixed greenhouse)
{
	fixed energy_per_meter2;
	if (primary->type == SBody::TYPE_GRAVPOINT) {
		// binary. take energies of both stars
		energy_per_meter2 = calcEnergyPerUnitAreaAtDist(primary->children[0]->radius,
			primary->children[0]->averageTemp, distToPrimary);
		energy_per_meter2 += calcEnergyPerUnitAreaAtDist(primary->children[1]->radius,
			primary->children[1]->averageTemp, distToPrimary);
	} else {
		energy_per_meter2 = calcEnergyPerUnitAreaAtDist(primary->radius, primary->averageTemp, distToPrimary);
	}
	const fixed surface_temp_pow4 = energy_per_meter2*(1-albedo)/(1-greenhouse);
	return isqrt(isqrt((surface_temp_pow4.v>>fixed::FRAC)*4409673));
}

void Orbit::KeplerPosAtTime(double t, double *dist, double *ang)
{
	double e = eccentricity;
	double a = semiMajorAxis;
	// mean anomaly
	double M = 2*M_PI*t / period;
	// eccentric anomaly
	double E = M + (e - (1/8.0)*e*e*e)*sin(M) +
	               (1/2.0)*e*e*sin(2*M) +
		       (3/8.0)*e*e*e*sin(3*M);
	// true anomaly (angle of orbit position)
	double v = 2*atan(sqrt((1+e)/(1-e)) * tan(E/2.0));
	// heliocentric distance
	double r = a * (1 - e*e) / (1 + e*cos(v));
	*ang = v;
	*dist = r;
}
			
vector3d Orbit::CartesianPosAtTime(double t)
{
	double dist, ang;
	KeplerPosAtTime(t, &dist, &ang);
	vector3d pos = vector3d(cos(ang)*dist, sin(ang)*dist, 0);
	pos = rotMatrix * pos;
	return pos;
}

double calc_orbital_period(double semiMajorAxis, double centralMass)
{
	return 2.0*M_PI*sqrt((semiMajorAxis*semiMajorAxis*semiMajorAxis)/(G*centralMass));
}

SBodyPath::SBodyPath()
{
	sectorX = sectorY = systemIdx = 0;
	for (int i=0; i<SBODYPATHLEN; i++) elem[i] = -1;
}
SBodyPath::SBodyPath(int sectorX, int sectorY, int systemIdx)
{
	this->sectorX = sectorX;
	this->sectorY = sectorY;
	this->systemIdx = systemIdx;
	for (int i=0; i<SBODYPATHLEN; i++) elem[i] = -1;
}

void SBodyPath::Serialize() const
{
	using namespace Serializer::Write;
	wr_int(sectorX);
	wr_int(sectorY);
	wr_int(systemIdx);
	for (int i=0; i<SBODYPATHLEN; i++) wr_byte(elem[i]);
}

void SBodyPath::Unserialize(SBodyPath *path)
{
	using namespace Serializer::Read;
	path->sectorX = rd_int();
	path->sectorY = rd_int();
	path->systemIdx = rd_int();
	for (int i=0; i<SBODYPATHLEN; i++) path->elem[i] = rd_byte();
}

SBody *StarSystem::GetBodyByPath(const SBodyPath *path) const
{
	assert((m_secx == path->sectorX) || (m_secy == path->sectorY) ||
	       (m_sysIdx == path->systemIdx));

	SBody *body = rootBody;
	for (int i=0; i<SBODYPATHLEN; i++) {
		if (path->elem[i] == -1) continue;
		else {
			body = body->children[path->elem[i]];
		}
	}
	return body;
}

void StarSystem::GetPathOf(const SBody *sbody, SBodyPath *path) const
{
	*path = SBodyPath();

	int pos = SBODYPATHLEN-1;

	for (const SBody *parent = sbody->parent;;) {
		if (!parent) break;
		assert((pos>=0) && (pos < SBODYPATHLEN));

		// find position of sbody in parent
		unsigned int index = 0;
		bool found = false;
		for (; index < parent->children.size(); index++) {
			if (parent->children[index] == sbody) {
				assert(index < 128);
				path->elem[pos--] = index;
				sbody = parent;
				parent = parent->parent;
				found = true;
				break;
			}
		}
		assert(found);
	}
	path->sectorX = m_secx;
	path->sectorY = m_secy;
	path->systemIdx = m_sysIdx;
}

/*
struct CustomSBody {
	const char *name; // null to end system
	SBody::BodyType type;
	int primaryIdx;  // -1 for primary
	fixed radius; // in earth radii for planets, sol radii for stars
	fixed mass; // earth masses or sol masses
	int averageTemp; // kelvin
	fixed semiMajorAxis; // in AUs
	fixed eccentricity;
};
*/
void StarSystem::CustomGetKidsOf(SBody *parent, const CustomSBody *customDef, const int primaryIdx)
{
	const CustomSBody *c = customDef;
	for (int i=0; c->name; c++, i++) {
		if (c->primaryIdx != primaryIdx) continue;
		
		SBody *kid = new SBody;
		SBody::BodyType type = c->type;
		kid->seed = rand.Int32();
		kid->type = type;
		kid->parent = parent;
		kid->radius = c->radius;
		kid->mass = c->mass;
		kid->econType = c->econType;
		kid->averageTemp = c->averageTemp;
		kid->name = c->name;
		kid->rotationPeriod = c->rotationPeriod;
		kid->eccentricity = c->eccentricity;
		kid->semiMajorAxis = c->semiMajorAxis;
		kid->orbit.eccentricity = c->eccentricity.ToDouble();
		kid->orbit.semiMajorAxis = c->semiMajorAxis.ToDouble() * AU;
		kid->orbit.period = calc_orbital_period(kid->orbit.semiMajorAxis, parent->GetMass());
		kid->heightMapFilename = c->heightMapFilename;

		if (kid->type == SBody::TYPE_STARPORT_SURFACE) {
			kid->orbit.rotMatrix = matrix4x4d::RotateYMatrix(c->longitude) *
				matrix4x4d::RotateXMatrix(-0.5*M_PI + c->latitude);
		} else {
			kid->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rand.Double(2*M_PI)) *
				matrix4x4d::RotateXMatrix(-0.5*M_PI + c->latitude);
		}
		parent->children.push_back(kid);

		// perihelion and aphelion (in AUs)
		kid->orbMin = c->semiMajorAxis - c->eccentricity*c->semiMajorAxis;
		kid->orbMax = 2*c->semiMajorAxis - kid->orbMin;

		PickEconomicStuff(kid);
		CustomGetKidsOf(kid, customDef, i);
	}
}

void StarSystem::GenerateFromCustom(const CustomSystem *customSys)
{
	// find primary
	const CustomSBody *csbody = customSys->sbodies;

	int idx = 0;
	while ((csbody->name) && (csbody->primaryIdx != -1)) { csbody++; idx++; }
	assert(csbody->primaryIdx == -1);

	rootBody = new SBody;
	SBody::BodyType type = csbody->type;
	rootBody->type = type;
	rootBody->parent = NULL;
	rootBody->seed = rand.Int32();
	rootBody->radius = csbody->radius;
	rootBody->mass = csbody->mass;
	rootBody->averageTemp = csbody->averageTemp;
	rootBody->name = csbody->name;
	
	CustomGetKidsOf(rootBody, customSys->sbodies, idx);

}

void StarSystem::MakeStarOfType(SBody *sbody, SBody::BodyType type, MTRand &rand)
{
	sbody->type = type;
	sbody->radius = fixed(bodyTypeInfo[type].radius, 100);
	sbody->mass = fixed(rand.Int32(bodyTypeInfo[type].mass[0],
				bodyTypeInfo[type].mass[1]), 100);
	sbody->averageTemp = rand.Int32(bodyTypeInfo[type].tempMin,
				bodyTypeInfo[type].tempMax);
}

void StarSystem::MakeRandomStar(SBody *sbody, MTRand &rand)
{
	SBody::BodyType type = (SBody::BodyType)rand.Int32((int)SBody::TYPE_STAR_MIN, (int)SBody::TYPE_STAR_MAX);
	MakeStarOfType(sbody, type, rand);
}

void StarSystem::MakeStarOfTypeLighterThan(SBody *sbody, SBody::BodyType type, fixed maxMass, MTRand &rand)
{
	int tries = 16;
	do {
		MakeStarOfType(sbody, type, rand);
	} while ((sbody->mass > maxMass) && (--tries));
}

void StarSystem::MakeBinaryPair(SBody *a, SBody *b, fixed minDist, MTRand &rand)
{
	fixed m = a->mass + b->mass;
	fixed a0 = b->mass / m;
	fixed a1 = a->mass / m;
	a->eccentricity = rand.NFixed(3);
	int mul = 1;

	do {
		switch (rand.Int32(3)) {
			case 2: a->semiMajorAxis = fixed(rand.Int32(100,10000), 100); break;
			case 1: a->semiMajorAxis = fixed(rand.Int32(10,1000), 100); break;
			default:
			case 0: a->semiMajorAxis = fixed(rand.Int32(1,100), 100); break;
		}
		a->semiMajorAxis *= mul;
		mul *= 2;
	} while (a->semiMajorAxis < minDist);

	a->orbit.eccentricity = a->eccentricity.ToDouble();
	a->orbit.semiMajorAxis = AU * (a->semiMajorAxis * a0).ToDouble();
	a->orbit.period = 60*60*24*365* a->semiMajorAxis.ToDouble() * sqrt(a->semiMajorAxis.ToDouble() / m.ToDouble());
	
	const float rotY = (float)(rand.Double()*M_PI/2.0);
	const float rotZ = (float)rand.Double(M_PI);
	a->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY) * matrix4x4d::RotateZMatrix(rotZ);
	b->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY) * matrix4x4d::RotateZMatrix(rotZ-M_PI);

	b->orbit.eccentricity = a->eccentricity.ToDouble();
	b->orbit.semiMajorAxis = AU * (a->semiMajorAxis * a1).ToDouble();
	b->orbit.period = a->orbit.period;
	
	fixed orbMin = a->semiMajorAxis - a->eccentricity*a->semiMajorAxis;
	fixed orbMax = 2*a->semiMajorAxis - orbMin;
	a->orbMin = orbMin;
	b->orbMin = orbMin;
	a->orbMax = orbMax;
	b->orbMax = orbMax;
}

SBody::SBody()
{
	econType = 0;
	heightMapFilename = 0;
	memset(tradeLevel, 0, sizeof(tradeLevel));
}

/*
 * As my excellent comrades have pointed out, choices that depend on floating
 * point crap will result in different universes on different platforms.
 *
 * We must be sneaky and avoid floating point in these places.
 */
StarSystem::StarSystem(int sector_x, int sector_y, int system_idx)
{
	unsigned long _init[5] = { system_idx, sector_x, sector_y, UNIVERSE_SEED, 0 };
	m_secx = sector_x;
	m_secy = sector_y;
	m_sysIdx = system_idx;
	rootBody = 0;
	if (system_idx == -1) return;

	Sector s = Sector(sector_x, sector_y);
	_init[4] = s.m_systems[system_idx].seed;
	rand.seed(_init, 5);

	if (s.m_systems[system_idx].customSys) {
		const CustomSystem *custom = s.m_systems[system_idx].customSys;
		if (custom->shortDesc) m_shortDesc = custom->shortDesc;
		if (custom->longDesc) m_longDesc = custom->longDesc;
		if (custom->sbodies) {
			GenerateFromCustom(s.m_systems[system_idx].customSys);
			return;
		}
	}

	SBody *star[4];
	SBody *centGrav1, *centGrav2;

	const int numStars = s.m_systems[system_idx].numStars;
	assert((numStars >= 1) && (numStars <= 4));

	if (numStars == 1) {
		SBody::BodyType type = s.m_systems[system_idx].starType[0];
		star[0] = new SBody;
		star[0]->parent = NULL;
		star[0]->name = s.m_systems[system_idx].name;
		star[0]->orbMin = 0;
		star[0]->orbMax = 0;
		MakeStarOfType(star[0], type, rand);
		rootBody = star[0];
		m_numStars = 1;
	} else {
		centGrav1 = new SBody;
		centGrav1->type = SBody::TYPE_GRAVPOINT;
		centGrav1->parent = NULL;
		centGrav1->name = s.m_systems[system_idx].name+" A,B";
		rootBody = centGrav1;

		SBody::BodyType type = s.m_systems[system_idx].starType[0];
		star[0] = new SBody;
		star[0]->name = s.m_systems[system_idx].name+" A";
		star[0]->parent = centGrav1;
		MakeStarOfType(star[0], type, rand);
		
		star[1] = new SBody;
		star[1]->name = s.m_systems[system_idx].name+" B";
		star[1]->parent = centGrav1;
		MakeStarOfTypeLighterThan(star[1], s.m_systems[system_idx].starType[1],
				star[0]->mass, rand);

		centGrav1->mass = star[0]->mass + star[1]->mass;
		centGrav1->children.push_back(star[0]);
		centGrav1->children.push_back(star[1]);
try_that_again_guvnah:
		MakeBinaryPair(star[0], star[1], fixed(0), rand);

		m_numStars = 2;

		if (numStars > 2) {
			if (star[0]->orbMax > fixed(100,1)) {
				// reduce to < 100 AU...
				goto try_that_again_guvnah;
			}
			// 3rd and maybe 4th star
			if (numStars == 3) {
				star[2] = new SBody;
				star[2]->name = s.m_systems[system_idx].name+" C";
				star[2]->orbMin = 0;
				star[2]->orbMax = 0;
				MakeStarOfTypeLighterThan(star[2], s.m_systems[system_idx].starType[2],
					star[0]->mass, rand);
				centGrav2 = star[2];
				m_numStars = 3;
			} else {
				centGrav2 = new SBody;
				centGrav2->type = SBody::TYPE_GRAVPOINT;
				centGrav2->name = s.m_systems[system_idx].name+" C,D";
				centGrav2->orbMax = 0;

				star[2] = new SBody;
				star[2]->name = s.m_systems[system_idx].name+" C";
				star[2]->parent = centGrav2;
				MakeStarOfTypeLighterThan(star[2], s.m_systems[system_idx].starType[2],
					star[0]->mass, rand);
				
				star[3] = new SBody;
				star[3]->name = s.m_systems[system_idx].name+" D";
				star[3]->parent = centGrav2;
				MakeStarOfTypeLighterThan(star[3], s.m_systems[system_idx].starType[3],
					star[2]->mass, rand);

				MakeBinaryPair(star[2], star[3], fixed(0), rand);
				centGrav2->mass = star[2]->mass + star[3]->mass;
				centGrav2->children.push_back(star[2]);
				centGrav2->children.push_back(star[3]);
				m_numStars = 4;
			}
			SBody *superCentGrav = new SBody;
			superCentGrav->type = SBody::TYPE_GRAVPOINT;
			superCentGrav->parent = NULL;
			superCentGrav->name = s.m_systems[system_idx].name;
			centGrav1->parent = superCentGrav;
			centGrav2->parent = superCentGrav;
			rootBody = superCentGrav;
			const fixed minDist = star[0]->orbMax + star[2]->orbMax;
			MakeBinaryPair(centGrav1, centGrav2, 4*minDist, rand);
			superCentGrav->children.push_back(centGrav1);
			superCentGrav->children.push_back(centGrav2);

		}
	}

	{ /* decide how infested the joint is */
		const int dist = 1+MAX(abs(sector_x), abs(sector_y));
		m_humanInfested = (fixed(1,2)+fixed(1,2)*rand.Fixed()) / dist;
	}

	for (int i=0; i<m_numStars; i++) MakePlanetsAround(star[i]);

	if (m_numStars > 1) MakePlanetsAround(centGrav1);
	if (m_numStars == 4) MakePlanetsAround(centGrav2);

	rootBody->AddHumanStuff(this);
}

/*
 * http://en.wikipedia.org/wiki/Hill_sphere
 */
fixed SBody::CalcHillRadius() const
{
	if (GetSuperType() <= SUPERTYPE_STAR) {
		return fixed(0);
	} else {
		// playing with precision since these numbers get small
		// masses in earth masses
		fixedf<32> mprimary = parent->GetMassInEarths();

		fixedf<48> a = semiMajorAxis;
		fixedf<48> e = eccentricity;

		return fixed(a * (fixedf<48>(1,1)-e) *
				fixedf<48>::CubeRootOf(fixedf<48>(
						mass / (fixedf<32>(3,1)*mprimary))));
		
		//fixed hr = semiMajorAxis*(fixed(1,1) - eccentricity) *
		//  fixedcuberoot(mass / (3*mprimary));
	}
}

static fixed density_from_disk_area(fixed a, fixed b, fixed max)
{
	// so, density of the disk with distance from star goes like so: 1 - x/discMax
	//
	// --- 
	//    ---
	//       --- <- zero at discMax
	//
	// Which turned into a disc becomes x - (x*x)/discMax
	// Integral of which is: 0.5*x*x - (1/(3*discMax))*x*x*x
	//
	b = (b > max ? max : b);
	assert(b>=a);
	assert(a<=max);
	assert(b<=max);
	assert(a>=0);
	fixed one_over_3max = fixed(1,1)/(3*max);
	return (fixed(1,2)*b*b - one_over_3max*b*b*b) -
		(fixed(1,2)*a*a - one_over_3max*a*a*a);
}


void StarSystem::MakePlanetsAround(SBody *primary)
{
	fixed discMin = fixed(0);
	fixed discMax = fixed(5000,1);
	fixed discDensity = 20*rand.NFixed(4);

	SBody::BodySuperType superType = primary->GetSuperType();

	if (superType <= SBody::SUPERTYPE_STAR) {
		if (primary->type == SBody::TYPE_GRAVPOINT) {
			/* around a binary */
			discMin = primary->children[0]->orbMax * SAFE_DIST_FROM_BINARY;
		} else {
			/* correct thing is roche limit, but lets ignore that because
			 * it depends on body densities and gives some strange results */
			discMin = 4 * primary->radius * AU_SOL_RADIUS;
		}
		if (primary->type == SBody::TYPE_WHITE_DWARF) {
			// white dwarfs will have started as stars < 8 solar
			// masses or so, so pick discMax according to that
			discMax = 100 * rand.NFixed(2)*fixed::SqrtOf(fixed(1,2) + fixed(8,1)*rand.Fixed());
		} else {
			discMax = 100 * rand.NFixed(2)*fixed::SqrtOf(primary->mass);
		}

		if ((superType == SBody::SUPERTYPE_STAR) && (primary->parent)) {
			// limit planets out to 10% distance to star's binary companion
			discMax = primary->orbMin * fixed(1,10);
		}

		/* in trinary and quaternary systems don't bump into other pair... */
		if (m_numStars >= 3) {
			discMax = MIN(discMax, fixed(5,100)*rootBody->children[0]->orbMin);
		}
	} else {
		fixed primary_rad = primary->radius * AU_EARTH_RADIUS;
		discMin = 4 * primary_rad;
		/* use hill radius to find max size of moon system. for stars botch it */
		discMax = MIN(discMax, fixed(1,4)*primary->CalcHillRadius());
	}

	//printf("Around %s: Range %f -> %f AU\n", primary->name.c_str(), discMin.ToDouble(), discMax.ToDouble());

	fixed initialJump = rand.NFixed(5);
	fixed pos = (fixed(1,1) - initialJump)*discMin + (initialJump*discMax);

	while (pos < discMax) {
		// periapsis, apoapsis = closest, farthest distance in orbit
		fixed periapsis = pos + pos*0.5*rand.NFixed(2);/* + jump */;
		fixed ecc = rand.NFixed(3);
		fixed semiMajorAxis = periapsis / (fixed(1,1) - ecc);
		fixed apoapsis = 2*semiMajorAxis - periapsis;
		if (apoapsis > discMax) break;

		fixed mass;
		{
			const fixed a = pos;
			const fixed b = fixed(135,100)*apoapsis;
			mass = density_from_disk_area(a, b, discMax);
			mass *= rand.Fixed() * discDensity;
		}

		SBody *planet = new SBody;
		planet->eccentricity = ecc;
		planet->semiMajorAxis = semiMajorAxis;
		planet->type = SBody::TYPE_PLANET_DWARF;
		planet->seed = rand.Int32();
		planet->humanActivity = m_humanInfested * rand.Fixed();
		planet->tmp = 0;
		planet->parent = primary;
	//	planet->radius = EARTH_RADIUS*bodyTypeInfo[type].radius;
		planet->mass = mass;
		planet->rotationPeriod = fixed(rand.Int32(1,200), 24);

		planet->orbit.eccentricity = ecc.ToDouble();
		planet->orbit.semiMajorAxis = semiMajorAxis.ToDouble() * AU;
		planet->orbit.period = calc_orbital_period(planet->orbit.semiMajorAxis, primary->GetMass());
		planet->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rand.Double(2*M_PI)) *
			matrix4x4d::RotateXMatrix(-0.5*M_PI + rand.NDouble(5)*M_PI/2.0);
		planet->orbMin = periapsis;
		planet->orbMax = apoapsis;
		primary->children.push_back(planet);

		/* minimum separation between planets of 1.35 */
		pos = apoapsis * fixed(135,100);
	}

	int idx=0;
	bool make_moons = superType <= SBody::SUPERTYPE_STAR;
	
	for (std::vector<SBody*>::iterator i = primary->children.begin(); i != primary->children.end(); ++i) {
		// planets around a binary pair [gravpoint] -- ignore the stars...
		if ((*i)->GetSuperType() == SBody::SUPERTYPE_STAR) continue;
		// Turn them into something!!!!!!!
		char buf[8];
		if (superType <= SBody::SUPERTYPE_STAR) {
			// planet naming scheme
			snprintf(buf, sizeof(buf), " %c", 'b'+idx);
		} else {
			// moon naming scheme
			snprintf(buf, sizeof(buf), " %d", 1+idx);
		}
		(*i)->name = primary->name+buf;
		(*i)->PickPlanetType(this, rand);
		if (make_moons) MakePlanetsAround(*i);
		idx++;
	}
}

/*
 * For moons distance from star is not orbMin, orbMax.
 */
const SBody *SBody::FindStarAndTrueOrbitalRange(fixed &orbMin, fixed &orbMax)
{
	const SBody *planet = this;
	const SBody *star = this->parent;

	assert(star);

	/* while not found star yet.. */
	while (star->GetSuperType() > SBody::SUPERTYPE_STAR) {
		planet = star;
		star = star->parent;
	}

	orbMin = planet->orbMin;
	orbMax = planet->orbMax;
	return star;
}

void SBody::PickPlanetType(StarSystem *system, MTRand &rand)
{
	fixed albedo = rand.Fixed() * fixed(1,2);
	fixed globalwarming = rand.Fixed() * fixed(9,10);
	// light planets have bugger all atmosphere
	if (mass < 1) globalwarming *= mass;
	// big planets get high global warming due to thick atmos
	if (mass > 3) globalwarming *= (mass-2);
	globalwarming = CLAMP(globalwarming, fixed(0), fixed(95,100));

	fixed minDistToStar, maxDistToStar, averageDistToStar;
	const SBody *star = FindStarAndTrueOrbitalRange(minDistToStar, maxDistToStar);
	averageDistToStar = (minDistToStar+maxDistToStar)>>1;

	/* this is all of course a total fucking joke and un-physical */
	int bbody_temp;
	bool fiddle = false;
	for (int i=0; i<10; i++) {
		bbody_temp = CalcSurfaceTemp(star, averageDistToStar, albedo, globalwarming);
		//printf("temp %f, albedo %f, globalwarming %f\n", bbody_temp, albedo, globalwarming);
		// extreme high temperature and low mass causes atmosphere loss
#define ATMOS_LOSS_MASS_CUTOFF	2
#define ATMOS_TEMP_CUTOFF	400
#define FREEZE_TEMP_CUTOFF	220
		if ((bbody_temp > ATMOS_TEMP_CUTOFF) &&
		   (mass < ATMOS_LOSS_MASS_CUTOFF)) {
		//	printf("atmos loss\n");
			globalwarming = globalwarming * (mass/ATMOS_LOSS_MASS_CUTOFF);
			fiddle = true;
		}
		if (!fiddle) break;
		fiddle = false;
	}
	// this is utter rubbish. should decide atmosphere composition and then freeze out
	// components of it in the previous loop
	if ((bbody_temp < FREEZE_TEMP_CUTOFF) && (mass < 5)) {
		globalwarming *= 0.2;
		albedo = rand.Fixed()*fixed(5,100) + 0.9;
	}
	bbody_temp = CalcSurfaceTemp(star, averageDistToStar, albedo, globalwarming);
//	printf("= temp %f, albedo %f, globalwarming %f\n", bbody_temp, albedo, globalwarming);

	averageTemp = bbody_temp;
	econType = 0;

	if (mass > 317*13) {
		// more than 13 jupiter masses can fuse deuterium - is a brown dwarf
		type = SBody::TYPE_BROWN_DWARF;
		// XXX should prevent mass exceeding 65 jupiter masses or so,
		// when it becomes a star
	} else if (mass > 300) {
		type = SBody::TYPE_PLANET_LARGE_GAS_GIANT;
	} else if (mass > 90) {
		type = SBody::TYPE_PLANET_MEDIUM_GAS_GIANT;
	} else if (mass > 6) {
		type = SBody::TYPE_PLANET_SMALL_GAS_GIANT;
	} else {
		// terrestrial planets
		if (mass < fixed(1,20000)) {
			type = SBody::TYPE_PLANET_ASTEROID;
		} else if (mass < fixed(1, 15000)) {
			type = SBody::TYPE_PLANET_LARGE_ASTEROID;
		} else if (mass < fixed(2,1000)) {
			type = SBody::TYPE_PLANET_DWARF;
		} else if ((mass < fixed(2,10)) && (globalwarming < fixed(5,100))) {
			type = SBody::TYPE_PLANET_SMALL;
		} else if (mass < 3) {
			if ((averageTemp > CELSIUS-10) && (averageTemp < CELSIUS+70)) {
				// try for life
				int minTemp = CalcSurfaceTemp(star, maxDistToStar, albedo, globalwarming);
				int maxTemp = CalcSurfaceTemp(star, minDistToStar, albedo, globalwarming);

				if ((minTemp > CELSIUS-10) && (minTemp < CELSIUS+70) &&
				    (maxTemp > CELSIUS-10) && (maxTemp < CELSIUS+70)) {
					type = SBody::TYPE_PLANET_INDIGENOUS_LIFE;
				} else {
					type = SBody::TYPE_PLANET_WATER;
				}
			} else {
				if (rand.Int32(0,1)) type = SBody::TYPE_PLANET_CO2;
				else type = SBody::TYPE_PLANET_METHANE;
			}
		} else /* 3 < mass < 6 */ {
			if ((averageTemp > CELSIUS-10) && (averageTemp < CELSIUS+70)) {
				type = SBody::TYPE_PLANET_WATER_THICK_ATMOS;
			} else {
				if (rand.Int32(0,1)) type = SBody::TYPE_PLANET_CO2_THICK_ATMOS;
				else type = SBody::TYPE_PLANET_METHANE_THICK_ATMOS;
			}
		}
		// kind of crappy
		if ((mass > fixed(8,10)) && (!rand.Int32(0,15))) type = SBody::TYPE_PLANET_HIGHLY_VOLCANIC;
	}
	radius = fixed(bodyTypeInfo[type].radius, 100);
}

void StarSystem::PickEconomicStuff(SBody *b)
{
	int tries = rand.Int32(10, 20);
	// This is a fucking mess. why is econType being decided before
	// getting here...
	
	// things we produce, plus their inputs
	while (tries--) {
		Equip::Type t = static_cast<Equip::Type>(rand.Int32(Equip::FIRST_COMMODITY, Equip::LAST_COMMODITY));
		const EquipType &type = EquipType::types[t];
		if (!(type.econType & b->econType)) continue;
		// XXX techlevel??
		int howmuch = rand.Int32(1,5);
		b->tradeLevel[t] += -howmuch;
		for (int i=0; i<EQUIP_INPUTS; i++) {
			b->tradeLevel[type.inputs[i]] += howmuch;
		}
	}
	// Add a bunch of things people consume
	const int NUM_CONSUMABLES = 10;
	const Equip::Type consumables[NUM_CONSUMABLES] = { 
		Equip::AIR_PROCESSORS,
		Equip::GRAIN,
		Equip::FRUIT_AND_VEG,
		Equip::ANIMAL_MEAT,
		Equip::LIQUOR,
		Equip::CONSUMER_GOODS,
		Equip::MEDICINES,
		Equip::HAND_WEAPONS,
		Equip::NARCOTICS,
		Equip::LIQUID_OXYGEN
	};

	tries = rand.Int32(3,6);
	while (tries--) {
		Equip::Type t = consumables[rand.Int32(0, NUM_CONSUMABLES - 1)];
		if ((t == Equip::AIR_PROCESSORS) ||
		    (t == Equip::LIQUID_OXYGEN)) {
			if (b->type == SBody::TYPE_PLANET_INDIGENOUS_LIFE)
				continue;
		}
		if (b->tradeLevel[t] >= 0) {
			b->tradeLevel[t] += rand.Int32(1,4);
		}
	}
}

void SBody::AddHumanStuff(StarSystem *system)
{
	for (unsigned int i=0; i<children.size(); i++) {
		children[i]->AddHumanStuff(system);
	}

	unsigned long _init[5] = { system->m_sysIdx, system->m_secx,
			system->m_secy, UNIVERSE_SEED, this->seed };
	MTRand rand;
	rand.seed(_init, 5);
		
	fixed orbMax = fixed(1,4)*this->CalcHillRadius();
	fixed orbMin = 4 * this->radius * AU_EARTH_RADIUS;
	if (children.size()) orbMax = MIN(orbMax, fixed(1,2) * children[0]->orbMin);

	bool has_starports = false;
	// starports - orbital
	if ((orbMin < orbMax) && (averageTemp < CELSIUS+100) && (averageTemp > 100) &&
		(rand.Fixed() < humanActivity)) {

		has_starports = true;
		SBody *sp = new SBody;
		sp->type = SBody::TYPE_STARPORT_ORBITAL;
		sp->seed = rand.Int32();
		sp->tmp = 0;
		sp->econType = econType;
		sp->parent = this;
		sp->rotationPeriod = fixed(1,3600);
		sp->averageTemp = this->averageTemp;
		sp->mass = 0;
		sp->name = NameGenerator::Surname(rand) + " Spaceport";
		sp->humanActivity = humanActivity;
		/* just always plonk starports in near orbit */
		sp->semiMajorAxis = orbMin;
		sp->eccentricity = fixed(0);
		sp->orbit.eccentricity = 0;
		sp->orbit.semiMajorAxis = sp->semiMajorAxis.ToDouble()*AU;
		sp->orbit.period = calc_orbital_period(sp->orbit.semiMajorAxis, this->mass.ToDouble() * EARTH_MASS);
		sp->orbit.rotMatrix = matrix4x4d::Identity();
		children.insert(children.begin(), sp);
		sp->orbMin = sp->semiMajorAxis;
		sp->orbMax = sp->semiMajorAxis;

		if (rand.Fixed() < humanActivity) {
			SBody *sp2 = new SBody;
			*sp2 = *sp;
			sp2->orbit.rotMatrix = matrix4x4d::RotateZMatrix(M_PI);
			sp2->name = NameGenerator::Surname(rand) + " Spaceport";
			children.insert(children.begin(), sp2);
		}
	}
	// starports - surface
	if ((averageTemp < CELSIUS+80) && (averageTemp > 100) &&
		((type == SBody::TYPE_PLANET_DWARF) ||
		(type == SBody::TYPE_PLANET_SMALL) ||
		(type == SBody::TYPE_PLANET_WATER) ||
		(type == SBody::TYPE_PLANET_CO2) ||
		(type == SBody::TYPE_PLANET_METHANE) ||
		(type == SBody::TYPE_PLANET_INDIGENOUS_LIFE))) {

		fixed activ = humanActivity;
		if (type == SBody::TYPE_PLANET_INDIGENOUS_LIFE) activ *= 4;

		int max = 6;
		while ((max-- > 0) && (rand.Fixed() < activ)) {
			has_starports = true;
			SBody *sp = new SBody;
			sp->type = SBody::TYPE_STARPORT_SURFACE;
			sp->seed = rand.Int32();
			sp->tmp = 0;
			sp->parent = this;
			sp->averageTemp = this->averageTemp;
			sp->humanActivity = activ;
			sp->mass = 0;
			sp->name = NameGenerator::Surname(rand) + " Starport";
			position_settlement_on_planet(sp);
			children.insert(children.begin(), sp);
		}
	}

	if (has_starports) {
		if (type == SBody::TYPE_PLANET_INDIGENOUS_LIFE)
			econType |= ECON_AGRICULTURE;
		else
			econType |= ECON_MINING;
		if (rand.Int32(2)) econType |= ECON_INDUSTRY;
		else econType |= ECON_MINING;
		system->PickEconomicStuff(this);
	}
}

StarSystem::~StarSystem()
{
	if (rootBody) delete rootBody;
}

bool StarSystem::IsSystem(int sector_x, int sector_y, int system_idx)
{
	return (sector_x == m_secx) && (sector_y == m_secy) && (system_idx == m_sysIdx);
}

SBody::~SBody()
{
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end(); ++i) {
		delete (*i);
	}
}

void StarSystem::Serialize(StarSystem *s)
{
	using namespace Serializer::Write;
	if (s) {
		wr_byte(1);
		wr_int(s->m_secx);
		wr_int(s->m_secy);
		wr_int(s->m_sysIdx);
	} else {
		wr_byte(0);
	}
}

StarSystem *StarSystem::Unserialize()
{
	using namespace Serializer::Read;
	if (rd_byte()) {
		int sec_x = rd_int();
		int sec_y = rd_int();
		int sys_idx = rd_int();
		return new StarSystem(sec_x, sec_y, sys_idx);
	} else {
		return 0;
	}
}

