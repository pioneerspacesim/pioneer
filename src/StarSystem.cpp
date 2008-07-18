#include "StarSystem.h"
#include "Sector.h"

#define CELSIUS	273.15

// indexed by enum type turd  
float StarSystem::starColors[7][3] = {
	{ 1.0, 0.2, 0.0 }, // M
	{ 1.0, 0.6, 0.1 }, // K
	{ 1.0, 1.0, 0.4 }, // G
	{ 1.0, 1.0, 0.8 }, // F
	{ 1.0, 1.0, 1.0 }, // A
	{ 0.7, 0.7, 1.0 }, // B
	{ 1.0, 0.7, 1.0 } // O
};

static const struct SBodySubTypeInfo {
	float mass;
	float radius; // sol radii for stars, earth radii for planets
	const char *description;
	const char *icon;
	float tempMin, tempMax;
} bodyTypeInfo[StarSystem::TYPE_MAX] = {
	{
		0.4, 0.5, "Type 'M' red star",
		"icons/object_star_m.png",
		2000, 3500
	}, {
		0.8, 0.9, "Type 'K' orange star",
		"icons/object_star_k.png",
		3500, 5000
	}, { 
		1.1, 1.1, "Type 'G' yellow star",
		"icons/object_star_g.png",
		5000, 6000
	}, {
		1.7, 1.4, "Type 'F' white star",
		"icons/object_star_f.png",
		6000, 7500
	}, {
		3.1, 2.1, "Type 'A' hot white star",
		"icons/object_star_a.png",
		7500, 10000
	}, {
		18.0, 7.0, "Bright type 'B' blue star",
		"icons/object_star_b.png",
		10000, 30000
	}, {
		64.0, 16.0, "Hot, massive type 'O' blue star",
		"icons/object_star_o.png",
		30000, 60000
	}, {
		0, 0, "Brown dwarf sub-stellar object",
		"icons/object_brown_dwarf.png"
	}, {
		0, 3.9, "Small gas giant",
		"icons/object_planet_small_gas_giant.png"
	}, {
		0, 9.5, "Medium gas giant",
		"icons/object_planet_medium_gas_giant.png"
	}, {
		0, 11.1, "Large gas giant",
		"icons/object_planet_large_gas_giant.png"
	}, {
		0, 15.0, "Very large gas giant",
		"icons/object_planet_large_gas_giant.png"
	}, {
		0, 0.26, "Small, rocky dwarf planet", // moon radius
		"icons/object_planet_dwarf.png"
	}, {
		0, 0.52, "Small, rocky planet with a thin atmosphere", // mars radius
		"icons/object_planet_small.png"
	}, {
		0, 1.0, "Rocky planet with liquid water and a nitrogen atmosphere", // earth radius
		"icons/object_planet_water_n2.png"
	}, {
		0, 1.0, "Rocky planet with a carbon dioxide atmosphere",
		"icons/object_planet_co2.png"
	}, {
		0, 1.0, "Rocky planet with a methane atmosphere",
		"icons/object_planet_methane.png"
	}, {
		0, 1.0, "Rocky planet with liquid water and a thick nitrogen atmosphere",
		"icons/object_planet_water_n2.png"
	}, {
		0, 1.0, "Rocky planet with a thick carbon dioxide atmosphere",
		"icons/object_planet_co2.png"
	}, {
		0, 1.0, "Rocky planet with a thick methane atmosphere",
		"icons/object_planet_methane.png"
	}, {
		0, 1.0, "Highly volcanic world",
		"icons/object_planet_volcanic.png"
	}, {
		0, 1.0, "World with indigenous life and an oxygen atmosphere",
		"icons/object_planet_life.png"
	}
};

const char *StarSystem::SBody::GetAstroDescription()
{
	return bodyTypeInfo[type].description;
}

const char *StarSystem::SBody::GetIcon()
{
	return bodyTypeInfo[type].icon;
}

static const double boltzman_const = 5.6704e-8;

static double calcEnergyPerUnitAreaAtDist(double star_radius, double star_temp, double object_dist)
{
	const double total_solar_emission = boltzman_const * pow(star_temp,4) *
			4*M_PI*star_radius*star_radius;

	return total_solar_emission / (4*M_PI*object_dist*object_dist);
}

// bond albedo, not geometric
static double calcSurfaceTemp(double star_radius, double star_temp, double object_dist, double albedo, double greenhouse)
{
	const double energy_per_meter2 = calcEnergyPerUnitAreaAtDist(star_radius, star_temp, object_dist);
	const double surface_temp = pow(energy_per_meter2*(1-albedo)/(4*(1-greenhouse)*boltzman_const), 0.25);
	return surface_temp;
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

static std::vector<float> *AccreteDisc(int size, float density, MTRand &rand)
{
	std::vector<float> *disc = new std::vector<float>();

	for (int i=0; i<size; i++) {
		disc->push_back(density*rand(1.0));
	}


	for (int iter=0; iter<20; iter++) {
		for (int i=0; i<(signed)disc->size(); i++) {
			for (int d=ceil(sqrtf((*disc)[i])+(i/5)); d>0; d--) {
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
	// now check for overlapping & unacceptably close orbits. merge planets
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end(); ++i) {
		if ((*i)->temp) continue;

		for (std::vector<SBody*>::iterator j = children.begin(); j != children.end(); ++j) {
			if ((*j) == (*i)) continue;
			// don't eat anything bigger than self
			if ((*j)->mass > (*i)->mass) continue;
			double i_min = (*i)->radMin;
			double i_max = (*i)->radMax;
			double j_min = (*j)->radMin;
			double j_max = (*j)->radMax;
			bool eat = false;
			if ((*i)->orbit.semiMajorAxis > (*j)->orbit.semiMajorAxis) {
				if (i_min < j_max*1.2) eat = true;
			} else {
				if (i_max > j_min*0.8) eat = true;
			}
			if (eat) {
				(*i)->mass += (*j)->mass;
				(*j)->temp = 1;
			}
		}

	}

	// kill the eaten ones
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end();) {
		if ((*i)->temp) {
			i = children.erase(i);
		}
		else
			++i;
	}
}

StarSystem::StarSystem(int sector_x, int sector_y, int system_idx)
{
	unsigned long _init[3] = { system_idx, sector_x, sector_y };
	loc.secX = sector_x;
	loc.secY = sector_y;
	loc.sysIdx = system_idx;
	rootBody = 0;
	if (system_idx == -1) return;
	rand.seed(_init, 3);

	Sector s = Sector(sector_x, sector_y);
	// primary
	SBody *primary = new SBody;

	StarSystem::BodyType type = s.m_systems[system_idx].primaryStarClass;
	primary->type = type;
	primary->parent = NULL;
	primary->radius = SOL_RADIUS*bodyTypeInfo[type].radius;
	primary->mass = SOL_MASS*bodyTypeInfo[type].mass;
	primary->supertype = SUPERTYPE_STAR;
	primary->averageTemp = rand((int)bodyTypeInfo[type].tempMin,
				(int)bodyTypeInfo[type].tempMax);
	rootBody = primary;

	int disc_size = rand(6,100) + rand(60,140)*primary->type*primary->type;
	//printf("disc_size %.1fAU\n", disc_size/10.0);

	std::vector<float> *disc = AccreteDisc(disc_size, 0.1+rand(1.5), rand);
	for (unsigned int i=0; i<disc->size(); i++) {
		float mass = (*disc)[i];
		if (mass == 0) continue;

		SBody *planet = new SBody;
		planet->type = TYPE_PLANET_DWARF;
		planet->seed = rand.Int32();
		planet->temp = 0;
		planet->parent = primary;
	//	planet->radius = EARTH_RADIUS*bodyTypeInfo[type].radius;
		planet->mass = mass * EARTH_MASS;
		planet->orbit.eccentricity = rand.pdrand(3);
		planet->orbit.semiMajorAxis = ((i+1)*0.1)*AU;
		planet->orbit.period = calc_orbital_period(planet->orbit.semiMajorAxis, primary->mass);
		planet->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rand.pdrand(5)*M_PI/2.0) *
					  matrix4x4d::RotateZMatrix(rand(M_PI));
		primary->children.push_back(planet);

		double ang;
		planet->orbit.KeplerPosAtTime(0, &planet->radMin, &ang);
		planet->orbit.KeplerPosAtTime(planet->orbit.period*0.5, &planet->radMax, &ang);
//		printf("%f,%f\n", min/AU, max/AU);
//		printf("%f year orbital period\n", planet->orbit.period / (60*60*24*365));
	}
	delete disc;

	// merge children with overlapping or very close orbits
	primary->EliminateBadChildren();
	primary->name = s.m_systems[system_idx].name;

	int idx=0;
	for (std::vector<SBody*>::iterator i = primary->children.begin(); i != primary->children.end(); ++i) {
		// Turn them into something!!!!!!!
		char buf[3];
		buf[0] = ' ';
		buf[1] = 'b'+(idx++);
		buf[2] = 0;
		(*i)->name = primary->name+buf;
		double d = 0.5*((*i)->radMin + (*i)->radMax);
		(*i)->PickPlanetType(primary, d, rand, true);
	}
}

void StarSystem::SBody::PickPlanetType(SBody *star, double distToPrimary, MTRand &rand, bool genMoons)
{
	float emass = mass / EARTH_MASS;

//#if 0
//static double calcSurfaceTemp(double star_radius, double star_temp, double object_dist, double albedo, double greenhouse)
//#endif
	// surface temperature
	// http://en.wikipedia.org/wiki/Black_body
	const double d = distToPrimary;
	double albedo = rand(0.5);
	double globalwarming = rand(0.9);
	// light planets have bugger all atmosphere
	if (emass < 1) globalwarming *= emass;
	// big planets get high global warming due to thick atmos
	if (emass > 3) globalwarming *= (emass-2.0f);
	globalwarming = CLAMP(globalwarming, 0, 0.95);

	//printf("=====================\nDist %f, mass %f, albedo %f, globalwarming %f\n", d, emass, albedo, globalwarming);

	/* this is all of course a total fucking joke and un-physical */
	double bbody_temp;
	bool fiddle = false;
	for (int i=0; i<10; i++) {
		bbody_temp = calcSurfaceTemp(star->radius, star->averageTemp, d, albedo, globalwarming);
		//printf("temp %f, albedo %f, globalwarming %f\n", bbody_temp, albedo, globalwarming);
		// extreme high temperature and low mass causes atmosphere loss
#define ATMOS_LOSS_MASS_CUTOFF	2.0
#define ATMOS_TEMP_CUTOFF	400
#define FREEZE_TEMP_CUTOFF	220
		if ((bbody_temp > ATMOS_TEMP_CUTOFF) &&
		   (emass < ATMOS_LOSS_MASS_CUTOFF)) {
		//	printf("atmos loss\n");
			globalwarming = globalwarming * (emass/ATMOS_LOSS_MASS_CUTOFF);
			fiddle = true;
		}
		if (!fiddle) break;
		fiddle = false;
	}
	// this is utter rubbish. should decide atmosphere composition and then freeze out
	// components of it in the previous loop
	if ((bbody_temp < FREEZE_TEMP_CUTOFF) && (emass < 5)) {
		globalwarming *= 0.2;
		albedo = rand(0.05) + 0.9;
	}
	bbody_temp = calcSurfaceTemp(star->radius, star->averageTemp, d, albedo, globalwarming);
//	printf("= temp %f, albedo %f, globalwarming %f\n", bbody_temp, albedo, globalwarming);

	averageTemp = bbody_temp;

	if (emass > 317.8*13) {
		// more than 13 jupiter masses can fuse deuterium - is a brown dwarf
		type = TYPE_BROWN_DWARF;
		// XXX should prevent mass exceeding 65 jupiter masses or so,
		// when it becomes a star
	} else if (emass > 300) {
		type = TYPE_PLANET_LARGE_GAS_GIANT;
	} else if (emass > 90) {
		type = TYPE_PLANET_MEDIUM_GAS_GIANT;
	} else if (emass > 6) {
		type = TYPE_PLANET_SMALL_GAS_GIANT;
	} else {
		// terrestrial planets
		if (emass < 0.02) {
			type = TYPE_PLANET_DWARF;
		} else if ((emass < 0.2) && (globalwarming < 0.05)) {
			type = TYPE_PLANET_SMALL;
		} else if (emass < 3) {
			if ((averageTemp > CELSIUS-10) && (averageTemp < CELSIUS+70)) {
				// try for life
				double minTemp = calcSurfaceTemp(star->radius, star->averageTemp, radMax, albedo, globalwarming);
				double maxTemp = calcSurfaceTemp(star->radius, star->averageTemp, radMin, albedo, globalwarming);

				if ((minTemp > CELSIUS-10) && (minTemp < CELSIUS+70) &&
				    (maxTemp > CELSIUS-10) && (maxTemp < CELSIUS+70)) {
					type = TYPE_PLANET_INDIGENOUS_LIFE;
				} else {
					type = TYPE_PLANET_WATER;
				}
			} else {
				if (rand(0,1)) type = TYPE_PLANET_CO2;
				else type = TYPE_PLANET_METHANE;
			}
		} else /* 3 < emass < 6 */ {
			if ((averageTemp > CELSIUS-10) && (averageTemp < CELSIUS+70)) {
				type = TYPE_PLANET_WATER_THICK_ATMOS;
			} else {
				if (rand(0,1)) type = TYPE_PLANET_CO2_THICK_ATMOS;
				else type = TYPE_PLANET_METHANE_THICK_ATMOS;
			}
		}
		// kind of crappy
		if ((emass > 0.8) && (!rand(0,15))) type = TYPE_PLANET_HIGHLY_VOLCANIC;
	}
	radius = EARTH_RADIUS*bodyTypeInfo[type].radius;

	// generate moons
	if (genMoons) {
		std::vector<float> *disc = AccreteDisc(2*sqrt(emass), 0.001, rand);
		for (unsigned int i=0; i<disc->size(); i++) {
			float mass = (*disc)[i];
			if (mass == 0) continue;

			SBody *moon = new SBody;
			moon->type = TYPE_PLANET_DWARF;
			moon->seed = rand.Int32();
			moon->temp = 0;
			moon->parent = this;
		//	moon->radius = EARTH_RADIUS*bodyTypeInfo[type].radius;
			moon->mass = mass * EARTH_MASS;
			moon->orbit.eccentricity = rand.pdrand(3);
			moon->orbit.semiMajorAxis = ((i+1)*0.001)*AU;
			moon->orbit.period = calc_orbital_period(moon->orbit.semiMajorAxis, this->mass);
			moon->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rand.pdrand(5)*M_PI/2.0) *
						  matrix4x4d::RotateZMatrix(rand(M_PI));
			this->children.push_back(moon);

			double ang;
			moon->orbit.KeplerPosAtTime(0, &moon->radMin, &ang);
			moon->orbit.KeplerPosAtTime(moon->orbit.period*0.5, &moon->radMax, &ang);
	//		printf("%f,%f\n", min/AU, max/AU);
	//		printf("%f year orbital period\n", moon->orbit.period / (60*60*24*365));
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
			(*i)->PickPlanetType(star, d, rand, false);
		}
	}
}

StarSystem::~StarSystem()
{
	if (rootBody) delete rootBody;
}

bool StarSystem::IsSystem(int sector_x, int sector_y, int system_idx)
{
	return (sector_x == loc.secX) && (sector_y == loc.secY) && (system_idx == loc.sysIdx);
}

StarSystem::SBody::~SBody()
{
	for (std::vector<SBody*>::iterator i = children.begin(); i != children.end(); ++i) {
		delete (*i);
	}
}

