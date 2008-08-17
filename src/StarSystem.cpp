#include "StarSystem.h"
#include "Sector.h"
#include "custom_starsystems.h"

#define CELSIUS	273.15
#define DEBUG_DUMP

// indexed by enum type turd  
float StarSystem::starColors[][3] = {
	{ 0, 0, 0 }, // gravpoint
	{ 1.0, 0.2, 0.0 }, // M
	{ 1.0, 0.6, 0.1 }, // K
	{ 1.0, 1.0, 0.4 }, // G
	{ 1.0, 1.0, 0.8 }, // F
	{ 1.0, 1.0, 1.0 }, // A
	{ 0.7, 0.7, 1.0 }, // B
	{ 1.0, 0.7, 1.0 }, // O
	{ 0.4, 0.4, 0.8 } // white dwarf
};

static const struct SBodySubTypeInfo {
	StarSystem::BodySuperType supertype;
	int mass[2]; // min,max % sol for stars, unused for planets
	int radius; // % sol radii for stars, % earth radii for planets
	const char *description;
	const char *icon;
	int tempMin, tempMax;
} bodyTypeInfo[StarSystem::TYPE_MAX] = {
	{
		StarSystem::SUPERTYPE_NONE, {}, 0, "Shouldn't see this!",
	}, {
		StarSystem::SUPERTYPE_STAR,
		{10,47}, 50, "Type 'M' red star",
		"icons/object_star_m.png",
		2000, 3500
	}, {
		StarSystem::SUPERTYPE_STAR,
		{50,78}, 90, "Type 'K' orange star",
		"icons/object_star_k.png",
		3500, 5000
	}, { 
		StarSystem::SUPERTYPE_STAR,
		{80,110}, 110, "Type 'G' yellow star",
		"icons/object_star_g.png",
		5000, 6000
	}, {
		StarSystem::SUPERTYPE_STAR,
		{115,170}, 140, "Type 'F' white star",
		"icons/object_star_f.png",
		6000, 7500
	}, {
		StarSystem::SUPERTYPE_STAR,
		{180,320}, 210, "Type 'A' hot white star",
		"icons/object_star_a.png",
		7500, 10000
	}, {
		StarSystem::SUPERTYPE_STAR,
		{400,1800}, 700, "Bright type 'B' blue star",
		"icons/object_star_b.png",
		10000, 30000
	}, {
		StarSystem::SUPERTYPE_STAR,
		{2000,4000}, 1600, "Hot, massive type 'O' blue star",
		"icons/object_star_o.png",
		30000, 60000
	}, {
		StarSystem::SUPERTYPE_STAR,
		{20,100}, 1, "White dwarf",
		"icons/object_white_dwarf.png",
		4000, 40000
	}, {
		StarSystem::SUPERTYPE_GAS_GIANT,
		{}, 30, "Brown dwarf sub-stellar object",
		"icons/object_brown_dwarf.png"
	}, {
		StarSystem::SUPERTYPE_GAS_GIANT,
		{}, 390, "Small gas giant",
		"icons/object_planet_small_gas_giant.png"
	}, {
		StarSystem::SUPERTYPE_GAS_GIANT,
		{}, 950, "Medium gas giant",
		"icons/object_planet_medium_gas_giant.png"
	}, {
		StarSystem::SUPERTYPE_GAS_GIANT,
		{}, 1110, "Large gas giant",
		"icons/object_planet_large_gas_giant.png"
	}, {
		StarSystem::SUPERTYPE_GAS_GIANT,
		{}, 1500, "Very large gas giant",
		"icons/object_planet_large_gas_giant.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 26, "Small, rocky dwarf planet", // moon radius
		"icons/object_planet_dwarf.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 52, "Small, rocky planet with a thin atmosphere", // mars radius
		"icons/object_planet_small.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with liquid water and a nitrogen atmosphere", // earth radius
		"icons/object_planet_water_n2.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a carbon dioxide atmosphere",
		"icons/object_planet_co2.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a methane atmosphere",
		"icons/object_planet_methane.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with liquid water and a thick nitrogen atmosphere",
		"icons/object_planet_water_n2.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a thick carbon dioxide atmosphere",
		"icons/object_planet_co2.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a thick methane atmosphere",
		"icons/object_planet_methane.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Highly volcanic world",
		"icons/object_planet_volcanic.png"
	}, {
		StarSystem::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "World with indigenous life and an oxygen atmosphere",
		"icons/object_planet_life.png"
	}
};

StarSystem::BodySuperType StarSystem::SBody::GetSuperType() const
{
	return bodyTypeInfo[type].supertype;
}

const char *StarSystem::SBody::GetAstroDescription()
{
	return bodyTypeInfo[type].description;
}

const char *StarSystem::SBody::GetIcon()
{
	return bodyTypeInfo[type].icon;
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

static int CalcSurfaceTemp(StarSystem::SBody *primary, fixed distToPrimary, fixed albedo, fixed greenhouse)
{
	fixed energy_per_meter2;
	if (primary->type == StarSystem::TYPE_GRAVPOINT) {
		// binary. take energies of both stars
		energy_per_meter2 = calcEnergyPerUnitAreaAtDist(primary->children[0]->radius,
			primary->children[0]->averageTemp, distToPrimary);
		energy_per_meter2 += calcEnergyPerUnitAreaAtDist(primary->children[1]->radius,
			primary->children[1]->averageTemp, distToPrimary);
	} else {
		energy_per_meter2 = calcEnergyPerUnitAreaAtDist(primary->radius, primary->averageTemp, distToPrimary);
	}
	const fixed surface_temp_pow4 = energy_per_meter2*(1-albedo)/(1-greenhouse);
	return isqrt(isqrt((surface_temp_pow4.v>>16)*4409673));
}

void StarSystem::Orbit::KeplerPosAtTime(double t, double *dist, double *ang)
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
			
vector3d StarSystem::Orbit::CartesianPosAtTime(double t)
{
	double dist, ang;
	KeplerPosAtTime(t, &dist, &ang);
	vector3d pos = vector3d(cos(ang)*dist, sin(ang)*dist, 0);
	pos = rotMatrix * pos;
	return pos;
}

static std::vector<int> *AccreteDisc(int size, int bandSize, int density, MTRand &rand)
{
	std::vector<int> *disc = new std::vector<int>(size);

	int bandDensity = 0;
	for (int i=0; i<size; i++) {
		if (!(i%bandSize)) bandDensity = rand.Int32(density);
		(*disc)[i] = bandDensity * rand.Int32(density);
	}

	for (int iter=0; iter<20; iter++) {
		for (int i=0; i<(signed)disc->size(); i++) {
			int d=1+(i/3);

			for (; d>0; d--) {
				if ((i+d < (signed)disc->size()) && ((*disc)[i] > (*disc)[i+d])) {
					(*disc)[i] += (*disc)[i+d];
					(*disc)[i+d] = 0;
				}
				if (((i-d) >= 0) && ((*disc)[i] > (*disc)[i-d])) {
					(*disc)[i] += (*disc)[i-d];
					(*disc)[i-d] = 0;
				}
			}
		}
	}
	return disc;
}

double calc_orbital_period(double semiMajorAxis, double centralMass)
{
	return 2.0*M_PI*sqrt((semiMajorAxis*semiMajorAxis*semiMajorAxis)/(G*centralMass));
}

void StarSystem::SBody::EliminateBadChildren()
{
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->tmp = 0;
	}
	// now check for overlapping & unacceptably close orbits. merge planets
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end(); ++i) {
		if ((*i)->GetSuperType() == SUPERTYPE_STAR) continue;
		if ((*i)->tmp) continue;

		for (std::vector<SBody*>::iterator j = children.begin(); j != children.end(); ++j) {
			if ((*j)->GetSuperType() == SUPERTYPE_STAR) continue;
			if ((*j) == (*i)) continue;
			// don't eat anything bigger than self
			if ((*j)->mass > (*i)->mass) continue;
			fixed i_min = (*i)->orbMin;
			fixed i_max = (*i)->orbMax;
			fixed j_min = (*j)->orbMin;
			fixed j_max = (*j)->orbMax;
			fixed i_avg = (i_min+i_max)>>1;
			fixed j_avg = (j_min+j_max)>>1;
			bool eat = false;
			if (i_avg > j_avg) {
				if (i_min < j_max*fixed(13,10)) eat = true;
			} else {
				if (i_max > j_min*fixed(7,10)) eat = true;
			}
			if (eat) {
				(*i)->mass += (*j)->mass;
				(*j)->tmp = 1;
			}
		}

	}

	// kill the eaten ones
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end();) {
		if ((*i)->tmp) {
			i = children.erase(i);
		}
		else
			++i;
	}
}

/*
struct CustomSBody {
	const char *name; // null to end system
	StarSystem::BodyType type;
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
		StarSystem::BodyType type = c->type;
		kid->type = type;
		kid->parent = parent;
		kid->radius = c->radius;
		kid->mass = c->mass;
		kid->averageTemp = c->averageTemp;
		kid->name = c->name;
		kid->rotationPeriod = c->rotationPeriod;
		
		kid->orbit.eccentricity = c->eccentricity.ToDouble();
		kid->orbit.semiMajorAxis = c->semiMajorAxis.ToDouble() * AU;
		kid->orbit.period = calc_orbital_period(kid->orbit.semiMajorAxis, parent->GetMass());
		kid->orbit.rotMatrix = matrix4x4d::RotateYMatrix(c->inclination) *
					  matrix4x4d::RotateZMatrix(rand.Double(M_PI));
		parent->children.push_back(kid);

		// perihelion and aphelion (in AUs)
		kid->orbMin = c->semiMajorAxis - c->eccentricity*c->semiMajorAxis;
		kid->orbMax = 2*c->semiMajorAxis - kid->orbMin;

		CustomGetKidsOf(kid, customDef, i);
	}
}

void StarSystem::GenerateFromCustom(const CustomSBody *customDef)
{
	// find primary
	const CustomSBody *csbody = customDef;

	int idx = 0;
	while ((csbody->name) && (csbody->primaryIdx != -1)) { csbody++; idx++; }
	assert(csbody->primaryIdx == -1);

	rootBody = new SBody;
	StarSystem::BodyType type = csbody->type;
	rootBody->type = type;
	rootBody->parent = NULL;
	rootBody->radius = csbody->radius;
	rootBody->mass = csbody->mass;
	rootBody->averageTemp = csbody->averageTemp;
	rootBody->name = csbody->name;
	
	CustomGetKidsOf(rootBody, customDef, idx);

}

void StarSystem::MakeStarOfType(SBody *sbody, BodyType type, MTRand &rand)
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
	BodyType type = (BodyType)rand.Int32((int)TYPE_STAR_MIN, (int)TYPE_STAR_MAX);
	MakeStarOfType(sbody, type, rand);
}

void StarSystem::MakeRandomStarLighterThan(SBody *sbody, fixed maxMass, MTRand &rand)
{
	do {
		MakeRandomStar(sbody, rand);
	} while (sbody->mass > maxMass);
}

void StarSystem::MakeBinaryPair(SBody *a, SBody *b, fixed minDist, MTRand &rand)
{
	fixed ecc = rand.NFixed(3);
	fixed m = a->mass + b->mass;
	fixed a0 = b->mass / m;
	fixed a1 = a->mass / m;
	fixed semiMajorAxis;
	int mul = 1;

	do {
		switch (rand.Int32(3)) {
			case 2: semiMajorAxis = fixed(rand.Int32(100,10000), 100); break;
			case 1: semiMajorAxis = fixed(rand.Int32(10,1000), 100); break;
			default:
			case 0: semiMajorAxis = fixed(rand.Int32(1,100), 100); break;
		}
		semiMajorAxis *= mul;
		mul *= 2;
	} while (semiMajorAxis < minDist);

	a->orbit.eccentricity = ecc.ToDouble();
	a->orbit.semiMajorAxis = AU * (semiMajorAxis * a0).ToDouble();
	a->orbit.period = 60*60*24*365* semiMajorAxis.ToDouble() * sqrt(semiMajorAxis.ToDouble() / m.ToDouble());
	
	const float rotY = rand.Double()*M_PI/2.0;
	const float rotZ = rand.Double(M_PI);
	a->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY) * matrix4x4d::RotateZMatrix(rotZ);
	b->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY) * matrix4x4d::RotateZMatrix(rotZ-M_PI);

	b->orbit.eccentricity = ecc.ToDouble();
	b->orbit.semiMajorAxis = AU * (semiMajorAxis * a1).ToDouble();
	b->orbit.period = a->orbit.period;
	
	fixed orbMin = semiMajorAxis - ecc*semiMajorAxis;
	fixed orbMax = 2*semiMajorAxis - orbMin;
	a->orbMin = orbMin;
	b->orbMin = orbMin;
	a->orbMax = orbMax;
	b->orbMax = orbMax;
}

/*
 * As my excellent comrades have pointed out, choices that depend on floating
 * point crap will result in different universes on different platforms.
 *
 * We must be sneaky and avoid floating point in these places.
 */
StarSystem::StarSystem(int sector_x, int sector_y, int system_idx)
{
	unsigned long _init[4] = { system_idx, sector_x, sector_y, UNIVERSE_SEED };
	m_secx = sector_x;
	m_secy = sector_y;
	m_sysIdx = system_idx;
	rootBody = 0;
	if (system_idx == -1) return;
	rand.seed(_init, 4);

	Sector s = Sector(sector_x, sector_y);

	if (s.m_systems[system_idx].customDef) {
		GenerateFromCustom(s.m_systems[system_idx].customDef);
		return;
	}

	SBody *star[4];
	SBody *centGrav1, *centGrav2;

	int isBinary = rand.Int32(2);
	if (!isBinary) {
		StarSystem::BodyType type = s.m_systems[system_idx].primaryStarClass;
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
		centGrav1->type = TYPE_GRAVPOINT;
		centGrav1->parent = NULL;
		centGrav1->name = s.m_systems[system_idx].name+" A,B";
		rootBody = centGrav1;

		StarSystem::BodyType type = s.m_systems[system_idx].primaryStarClass;
		star[0] = new SBody;
		star[0]->name = s.m_systems[system_idx].name+" A";
		star[0]->parent = centGrav1;
		MakeStarOfType(star[0], type, rand);
		
		star[1] = new SBody;
		star[1]->name = s.m_systems[system_idx].name+" B";
		star[1]->parent = centGrav1;
		MakeRandomStarLighterThan(star[1], star[0]->mass, rand);

		MakeBinaryPair(star[0], star[1], fixed(0), rand);

		centGrav1->mass = star[0]->mass + star[1]->mass;
		centGrav1->children.push_back(star[0]);
		centGrav1->children.push_back(star[1]);
		m_numStars = 2;

		if ((star[0]->orbMax < fixed(100,1)) &&
		    (!rand.Int32(3))) {
			// 3rd and maybe 4th star
			if (!rand.Int32(2)) {
				star[2] = new SBody;
				star[2]->name = s.m_systems[system_idx].name+" C";
				star[2]->orbMin = 0;
				star[2]->orbMax = 0;
				MakeRandomStarLighterThan(star[2], star[0]->mass, rand);
				centGrav2 = star[2];
				m_numStars = 3;
			} else {
				centGrav2 = new SBody;
				centGrav2->type = TYPE_GRAVPOINT;
				centGrav2->name = s.m_systems[system_idx].name+" C,D";
				centGrav2->orbMax = 0;

				star[2] = new SBody;
				star[2]->name = s.m_systems[system_idx].name+" C";
				star[2]->parent = centGrav2;
				MakeRandomStarLighterThan(star[2], star[0]->mass, rand);
				
				star[3] = new SBody;
				star[3]->name = s.m_systems[system_idx].name+" D";
				star[3]->parent = centGrav2;
				MakeRandomStarLighterThan(star[3], star[2]->mass, rand);

				MakeBinaryPair(star[2], star[3], fixed(0), rand);
				centGrav2->mass = star[2]->mass + star[3]->mass;
				centGrav2->children.push_back(star[2]);
				centGrav2->children.push_back(star[3]);
				m_numStars = 4;
			}
			SBody *superCentGrav = new SBody;
			superCentGrav->type = TYPE_GRAVPOINT;
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

	for (int i=0; i<m_numStars; i++) MakePlanetsAround(star[i]);

	if (m_numStars > 1) MakePlanetsAround(centGrav1);
	if (m_numStars == 4) MakePlanetsAround(centGrav2);
}

void StarSystem::MakePlanetsAround(SBody *primary)
{
	int disc_size = rand.Int32(6,100) + rand.Int32(60,140)*(10*primary->mass*primary->mass).ToInt64();
	//printf("disc_size %.1fAU\n", disc_size/10.0);
	
	// some restrictions on planet formation due to binary star orbits
	fixed orbMinKill = fixed(0);
	fixed orbMaxKill = fixed(disc_size, 10);
	if (primary->type == TYPE_GRAVPOINT) {
		SBody *star = primary->children[0];
		orbMinKill = star->orbMax*10;
	}
	else if ((primary->GetSuperType() == SUPERTYPE_STAR) && (primary->parent)) {
		// limit planets out to 10% distance to star's binary companion
		orbMaxKill = primary->orbMin * fixed(1,10);
	}
	if (m_numStars >= 3) {
		orbMaxKill = MIN(orbMaxKill, fixed(5,100)*rootBody->children[0]->orbMin);
	}

	std::vector<int> *disc = AccreteDisc(disc_size, 10, rand.Int32(10,400), rand);
	for (unsigned int i=0; i<disc->size(); i++) {
		fixed mass = fixed((*disc)[i]);
		if (mass == 0) continue;
		fixed semiMajorAxis = fixed(i+1, 10); // in AUs
		fixed ecc = rand.NFixed(3);
		// perihelion and aphelion (in AUs)
		fixed orbMin = semiMajorAxis - ecc*semiMajorAxis;
		fixed orbMax = 2*semiMajorAxis - orbMin;

		if ((orbMin < orbMinKill) ||
		    (orbMax > orbMaxKill)) continue;
		
		SBody *planet = new SBody;
		planet->type = TYPE_PLANET_DWARF;
		planet->seed = rand.Int32();
		planet->tmp = 0;
		planet->parent = primary;
	//	planet->radius = EARTH_RADIUS*bodyTypeInfo[type].radius;
		planet->mass = mass;
		planet->rotationPeriod = fixed(rand.Int32(1,200), 24);

		planet->orbit.eccentricity = ecc.ToDouble();
		planet->orbit.semiMajorAxis = semiMajorAxis.ToDouble() * AU;
		planet->orbit.period = calc_orbital_period(planet->orbit.semiMajorAxis, SOL_MASS*primary->mass.ToDouble());
		planet->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rand.NDouble(5)*M_PI/2.0) *
					  matrix4x4d::RotateZMatrix(rand.Double(M_PI));
		planet->orbMin = orbMin;
		planet->orbMax = orbMax;
		primary->children.push_back(planet);
	}
	delete disc;

	// merge children with overlapping or very close orbits
	primary->EliminateBadChildren();
	int idx=0;
	
	for (std::vector<SBody*>::iterator i = primary->children.begin(); i != primary->children.end(); ++i) {
		if ((*i)->GetSuperType() == SUPERTYPE_STAR) continue;
		// Turn them into something!!!!!!!
		char buf[3];
		buf[0] = ' ';
		buf[1] = 'b'+(idx++);
		buf[2] = 0;
		(*i)->name = primary->name+buf;
		fixed d = ((*i)->orbMin + (*i)->orbMax) >> 1;
		(*i)->PickPlanetType(primary, d, rand, true);

#ifdef DEBUG_DUMP
//		printf("%s: mass %f, semi-major axis %fAU, ecc %f\n", (*i)->name.c_str(), (*i)->mass.ToDouble(), (*i)->orbit.semiMajorAxis/AU, (*i)->orbit.eccentricity);
#endif /* DEBUG_DUMP */

	}
}

void StarSystem::SBody::PickPlanetType(SBody *star, const fixed distToPrimary, MTRand &rand, bool genMoons)
{
	fixed albedo = rand.Fixed() * fixed(1,2);
	fixed globalwarming = rand.Fixed() * fixed(9,10);
	// light planets have bugger all atmosphere
	if (mass < 1) globalwarming *= mass;
	// big planets get high global warming due to thick atmos
	if (mass > 3) globalwarming *= (mass-2);
	globalwarming = CLAMP(globalwarming, fixed(0), fixed(95,100));

	/* this is all of course a total fucking joke and un-physical */
	int bbody_temp;
	bool fiddle = false;
	for (int i=0; i<10; i++) {
		bbody_temp = CalcSurfaceTemp(star, distToPrimary, albedo, globalwarming);
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
		albedo = rand.Double(0.05) + 0.9;
	}
	bbody_temp = CalcSurfaceTemp(star, distToPrimary, albedo, globalwarming);
//	printf("= temp %f, albedo %f, globalwarming %f\n", bbody_temp, albedo, globalwarming);

	averageTemp = bbody_temp;

	if (mass > 317*13) {
		// more than 13 jupiter masses can fuse deuterium - is a brown dwarf
		type = TYPE_BROWN_DWARF;
		// XXX should prevent mass exceeding 65 jupiter masses or so,
		// when it becomes a star
	} else if (mass > 300) {
		type = TYPE_PLANET_LARGE_GAS_GIANT;
	} else if (mass > 90) {
		type = TYPE_PLANET_MEDIUM_GAS_GIANT;
	} else if (mass > 6) {
		type = TYPE_PLANET_SMALL_GAS_GIANT;
	} else {
		// terrestrial planets
		if (mass < fixed(2,100)) {
			type = TYPE_PLANET_DWARF;
		} else if ((mass < fixed(2,10)) && (globalwarming < fixed(5,100))) {
			type = TYPE_PLANET_SMALL;
		} else if (mass < 3) {
			if ((averageTemp > CELSIUS-10) && (averageTemp < CELSIUS+70)) {
				// try for life
				int minTemp = CalcSurfaceTemp(star, orbMax, albedo, globalwarming);
				int maxTemp = CalcSurfaceTemp(star, orbMin, albedo, globalwarming);

				if ((minTemp > CELSIUS-10) && (minTemp < CELSIUS+70) &&
				    (maxTemp > CELSIUS-10) && (maxTemp < CELSIUS+70)) {
					type = TYPE_PLANET_INDIGENOUS_LIFE;
				} else {
					type = TYPE_PLANET_WATER;
				}
			} else {
				if (rand.Int32(0,1)) type = TYPE_PLANET_CO2;
				else type = TYPE_PLANET_METHANE;
			}
		} else /* 3 < mass < 6 */ {
			if ((averageTemp > CELSIUS-10) && (averageTemp < CELSIUS+70)) {
				type = TYPE_PLANET_WATER_THICK_ATMOS;
			} else {
				if (rand.Int32(0,1)) type = TYPE_PLANET_CO2_THICK_ATMOS;
				else type = TYPE_PLANET_METHANE_THICK_ATMOS;
			}
		}
		// kind of crappy
		if ((mass > fixed(8,10)) && (!rand.Int32(0,15))) type = TYPE_PLANET_HIGHLY_VOLCANIC;
	}
	radius = fixed(bodyTypeInfo[type].radius, 100);

	// generate moons
	if ((genMoons) && (mass > fixed(1,2))) {
		std::vector<int> *disc = AccreteDisc(isqrt(mass.v>>13), 10, rand.Int32(1,10), rand);
		for (unsigned int i=0; i<disc->size(); i++) {
			fixed mass = fixed((*disc)[i]);
			if (mass == 0) continue;

			SBody *moon = new SBody;
			moon->type = TYPE_PLANET_DWARF;
			moon->seed = rand.Int32();
			moon->tmp = 0;
			moon->parent = this;
		//	moon->radius = EARTH_RADIUS*bodyTypeInfo[type].radius;
			moon->rotationPeriod = fixed(rand.Int32(1,200), 24);

			moon->mass = mass;
			fixed ecc = rand.NFixed(3);
			fixed semiMajorAxis = fixed(i+2, 2000);
			moon->orbit.eccentricity = ecc.ToDouble();
			moon->orbit.semiMajorAxis = semiMajorAxis.ToDouble()*AU;
			moon->orbit.period = calc_orbital_period(moon->orbit.semiMajorAxis, this->mass.ToDouble() * EARTH_MASS);
			moon->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rand.NDouble(5)*M_PI/2.0) *
						  matrix4x4d::RotateZMatrix(rand.Double(M_PI));
			this->children.push_back(moon);

			moon->orbMin = semiMajorAxis - ecc*semiMajorAxis;
			moon->orbMax = 2*semiMajorAxis - moon->orbMin;
		}
		delete disc;
	
		// merge moons with overlapping or very close orbits
		EliminateBadChildren();

		int idx=0;
		for(std::vector<SBody*>::iterator i = children.begin(); i!=children.end(); ++i) {
			// Turn them into something!!!!!!!
			char buf[2];
			buf[0] = '1'+(idx++);
			buf[1] = 0;
			(*i)->name = name+buf;
			(*i)->PickPlanetType(star, distToPrimary, rand, false);
		}
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

StarSystem::SBody::~SBody()
{
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end(); ++i) {
		delete (*i);
	}
}

