// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StarSystemGenerator.h"
#include "LuaNameGen.h"
#include "Pi.h"
#include "Galaxy.h"
#include "Sector.h"
#include "Factions.h"
#include "Lang.h"

static const fixed SUN_MASS_TO_EARTH_MASS = fixed(332998,1); // XXX Duplication from StarSystem.cpp
// if binary stars have separation s, planets can have stable
// orbits at (0.5 * s * SAFE_DIST_FROM_BINARY)
static const fixed SAFE_DIST_FROM_BINARY = fixed(5,1);
// very crudely
static const fixed AU_SOL_RADIUS = fixed(305,65536);
static const fixed AU_EARTH_RADIUS = fixed(3, 65536); // XXX Duplication from StarSystem.cpp
static const fixed FIXED_PI = fixed(103993,33102); // XXX Duplication from StarSystem.cpp
static const double CELSIUS	= 273.15;

bool StarSystemFromSectorGenerator::Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config)
{
	PROFILE_SCOPED()
	RefCountedPtr<const Sector> sec = Pi::GetGalaxy()->GetSector(system->GetPath());
	assert(system->GetPath().systemIndex >= 0 && system->GetPath().systemIndex < sec->m_systems.size());
	const Sector::System& secSys = sec->m_systems[system->GetPath().systemIndex];

	system->SetFaction(Pi::GetGalaxy()->GetFactions()->GetNearestFaction(&secSys));
	system->SetSeed(secSys.GetSeed());
	system->SetName(secSys.GetName());
	system->SetUnexplored(!secSys.IsExplored());
	return true;
}


void StarSystemCustomGenerator::CustomGetKidsOf(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *parent,
	const std::vector<CustomSystemBody*> &children, int *outHumanInfestedness, Random &rand)
{
	PROFILE_SCOPED()
	// replaces gravpoint mass by sum of masses of its children
	// the code goes here to cover also planetary gravpoints (gravpoints that are not rootBody)
	if (parent->GetType() == SystemBody::TYPE_GRAVPOINT) {
		fixed mass(0);

		for (std::vector<CustomSystemBody*>::const_iterator i = children.begin(); i != children.end(); ++i) {
			const CustomSystemBody *csbody = *i;

			if (csbody->type >= SystemBody::TYPE_STAR_MIN && csbody->type <= SystemBody::TYPE_STAR_MAX)
				mass += csbody->mass;
			else
				mass += csbody->mass/SUN_MASS_TO_EARTH_MASS;
		}

		parent->m_mass = mass;
	}

	for (std::vector<CustomSystemBody*>::const_iterator i = children.begin(); i != children.end(); ++i) {
		const CustomSystemBody *csbody = *i;

		SystemBody *kid = system->NewBody();
		kid->m_type = csbody->type;
		kid->m_parent = parent;
		kid->m_seed = csbody->want_rand_seed ? rand.Int32() : csbody->seed;
		kid->m_radius = csbody->radius;
		kid->m_aspectRatio = csbody->aspectRatio;
		kid->m_averageTemp = csbody->averageTemp;
		kid->m_name = csbody->name;
		kid->m_isCustomBody = true;

		kid->m_mass = csbody->mass;
		if (kid->GetType() == SystemBody::TYPE_PLANET_ASTEROID) kid->m_mass /= 100000;

		kid->m_metallicity    = csbody->metallicity;
		//multiple of Earth's surface density
		kid->m_volatileGas    = csbody->volatileGas*fixed(1225,1000);
		kid->m_volatileLiquid = csbody->volatileLiquid;
		kid->m_volatileIces   = csbody->volatileIces;
		kid->m_volcanicity    = csbody->volcanicity;
		kid->m_atmosOxidizing = csbody->atmosOxidizing;
		kid->m_life           = csbody->life;

		kid->m_rotationPeriod = csbody->rotationPeriod;
		kid->m_rotationalPhaseAtStart = csbody->rotationalPhaseAtStart;
		kid->m_eccentricity = csbody->eccentricity;
		kid->m_orbitalOffset = csbody->orbitalOffset;
		kid->m_orbitalPhaseAtStart = csbody->orbitalPhaseAtStart;
		kid->m_axialTilt = csbody->axialTilt;
		kid->m_inclination = fixed(csbody->latitude*10000,10000);
		if(kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE)
			kid->m_orbitalOffset = fixed(csbody->longitude*10000,10000);
		kid->m_semiMajorAxis = csbody->semiMajorAxis;

		if (csbody->heightMapFilename.length() > 0) {
			kid->m_heightMapFilename = csbody->heightMapFilename;
			kid->m_heightMapFractal = csbody->heightMapFractal;
		}

		if(parent->GetType() == SystemBody::TYPE_GRAVPOINT) // generalize Kepler's law to multiple stars
			kid->m_orbit.SetShapeAroundBarycentre(csbody->semiMajorAxis.ToDouble() * AU, parent->GetMass(), kid->GetMass(), csbody->eccentricity.ToDouble());
		else
			kid->m_orbit.SetShapeAroundPrimary(csbody->semiMajorAxis.ToDouble() * AU, parent->GetMass(), csbody->eccentricity.ToDouble());

		kid->m_orbit.SetPhase(csbody->orbitalPhaseAtStart.ToDouble());

		if (kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
			kid->m_orbit.SetPlane(matrix3x3d::RotateY(csbody->longitude) * matrix3x3d::RotateX(-0.5*M_PI + csbody->latitude));
		} else {
			if (kid->m_orbit.GetSemiMajorAxis() < 1.2 * parent->GetRadius()) {
				Error("%s's orbit is too close to its parent", csbody->name.c_str());
			}
			double offset = csbody->want_rand_offset ? rand.Double(2*M_PI) : (csbody->orbitalOffset.ToDouble());
			kid->m_orbit.SetPlane(matrix3x3d::RotateY(offset) * matrix3x3d::RotateX(-0.5*M_PI + csbody->latitude));
		}
		if (kid->GetSuperType() == SystemBody::SUPERTYPE_STARPORT) {
			(*outHumanInfestedness)++;
            system->AddSpaceStation(kid);
		}
		parent->m_children.push_back(kid);

		// perihelion and aphelion (in AUs)
		kid->m_orbMin = csbody->semiMajorAxis - csbody->eccentricity*csbody->semiMajorAxis;
		kid->m_orbMax = 2*csbody->semiMajorAxis - kid->m_orbMin;

		PickAtmosphere(kid);

		// pick or specify rings
		switch (csbody->ringStatus) {
			case CustomSystemBody::WANT_NO_RINGS:
				kid->m_rings.minRadius = fixed();
				kid->m_rings.maxRadius = fixed();
				break;
			case CustomSystemBody::WANT_RINGS:
				PickRings(kid, true);
				break;
			case CustomSystemBody::WANT_RANDOM_RINGS:
				PickRings(kid, false);
				break;
			case CustomSystemBody::WANT_CUSTOM_RINGS:
				kid->m_rings.minRadius = csbody->ringInnerRadius;
				kid->m_rings.maxRadius = csbody->ringOuterRadius;
				kid->m_rings.baseColor = csbody->ringColor;
				break;
		}

		CustomGetKidsOf(system, kid, csbody->children, outHumanInfestedness, rand);
	}
}

bool StarSystemCustomGenerator::Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config)
{
	PROFILE_SCOPED()
	RefCountedPtr<const Sector> sec = Pi::GetGalaxy()->GetSector(system->GetPath());
	system->SetCustom(false, false);
	if (const CustomSystem *customSys = sec->m_systems[system->GetPath().systemIndex].GetCustomSystem()) {
		system->SetCustom(true, false);
		system->SetNumStars(customSys->numStars);
		if (customSys->shortDesc.length() > 0) system->SetShortDesc(customSys->shortDesc);
		if (customSys->longDesc.length() > 0) system->SetLongDesc(customSys->longDesc);
		if (!customSys->IsRandom()) {
			system->SetCustom(true, true);
			config->isCustomOnly = true;
			const CustomSystemBody *csbody = customSys->sBody;
			SystemBody* rootBody = system->NewBody();
			rootBody->m_type = csbody->type;
			rootBody->m_parent = 0;
			rootBody->m_seed = csbody->want_rand_seed ? rng.Int32() : csbody->seed;
			rootBody->m_seed = rng.Int32();
			rootBody->m_radius = csbody->radius;
			rootBody->m_aspectRatio = csbody->aspectRatio;
			rootBody->m_mass = csbody->mass;
			rootBody->m_averageTemp = csbody->averageTemp;
			rootBody->m_name = csbody->name;
			rootBody->m_isCustomBody = true;

			rootBody->m_rotationalPhaseAtStart = csbody->rotationalPhaseAtStart;
			rootBody->m_orbitalPhaseAtStart = csbody->orbitalPhaseAtStart;
			system->SetRootBody(rootBody);

			int humanInfestedness = 0;
			CustomGetKidsOf(system, rootBody, csbody->children, &humanInfestedness, rng);
			unsigned countedStars = 0;
			for (RefCountedPtr<SystemBody> b : system->GetBodies()) {
				if (b->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
					++countedStars;
					system->AddStar(b.Get());
				}
			}
			assert(countedStars == system->GetNumStars());

			return true;
		}
	}
	return true;
}


static fixed mass_from_disk_area(fixed a, fixed b, fixed max)
{
	PROFILE_SCOPED()
	// so, density of the disk with distance from star goes like so: 1 - x/discMax
	//
	// ---
	//    ---
	//       --- <- zero at discMax
	//
	// Which turned into a disc becomes 2*pi*x - (2*pi*x*x)/discMax
	// Integral of which is: pi*x*x - (2/(3*discMax))*pi*x*x*x
	//
	// Because get_disc_density divides total_mass by
	// mass_from_disk_area(0, discMax, discMax) to find density, the
	// constant factors (pi) in this equation drop out.
	//
	b = (b > max ? max : b);
	assert(b>=a);
	assert(a<=max);
	assert(b<=max);
	assert(a>=0);
	fixed one_over_3max = fixed(2,1)/(3*max);
	return (b*b - one_over_3max*b*b*b) -
		(a*a - one_over_3max*a*a*a);
}

static fixed get_disc_density(SystemBody *primary, fixed discMin, fixed discMax, fixed percentOfPrimaryMass)
{
	PROFILE_SCOPED()
	discMax = std::max(discMax, discMin);
	fixed total = mass_from_disk_area(discMin, discMax, discMax);
	return primary->GetMassInEarths() * percentOfPrimaryMass / total;
}

void StarSystemRandomGenerator::MakePlanetsAround(RefCountedPtr<StarSystem::GeneratorAPI> system, SystemBody *primary, Random &rand)
{
	PROFILE_SCOPED()
	fixed discMin = fixed();
	fixed discMax = fixed(5000,1);
	fixed discDensity;

	SystemBody::BodySuperType superType = primary->GetSuperType();

	if (superType <= SystemBody::SUPERTYPE_STAR) {
		if (primary->GetType() == SystemBody::TYPE_GRAVPOINT) {
			/* around a binary */
			discMin = primary->m_children[0]->m_orbMax * SAFE_DIST_FROM_BINARY;
		} else {
			/* correct thing is roche limit, but lets ignore that because
			 * it depends on body densities and gives some strange results */
			discMin = 4 * primary->GetRadiusAsFixed() * AU_SOL_RADIUS;
		}
		if (primary->GetType() == SystemBody::TYPE_WHITE_DWARF) {
			// white dwarfs will have started as stars < 8 solar
			// masses or so, so pick discMax according to that
			// We give it a larger discMin because it used to be a much larger star
			discMin = 1000 * primary->GetRadiusAsFixed() * AU_SOL_RADIUS;
			discMax = 100 * rand.NFixed(2);		// rand-splitting again
			discMax *= fixed::SqrtOf(fixed(1,2) + fixed(8,1)*rand.Fixed());
		} else {
			discMax = 100 * rand.NFixed(2)*fixed::SqrtOf(primary->GetMassAsFixed());
		}
		// having limited discMin by bin-separation/fake roche, and
		// discMax by some relation to star mass, we can now compute
		// disc density
		discDensity = rand.Fixed() * get_disc_density(primary, discMin, discMax, fixed(2,100));

		if ((superType == SystemBody::SUPERTYPE_STAR) && (primary->m_parent)) {
			// limit planets out to 10% distance to star's binary companion
			discMax = std::min(discMax, primary->m_orbMin * fixed(1,10));
		}

		/* in trinary and quaternary systems don't bump into other pair... */
		if (system->GetNumStars() >= 3) {
			discMax = std::min(discMax, fixed(5,100)*system->GetRootBody()->GetChildren()[0]->m_orbMin);
		}
	} else {
		fixed primary_rad = primary->GetRadiusAsFixed() * AU_EARTH_RADIUS;
		discMin = 4 * primary_rad;
		/* use hill radius to find max size of moon system. for stars botch it.
		   And use planets orbit around its primary as a scaler to a moon's orbit*/
		discMax = std::min(discMax, fixed(1,20)*
			CalcHillRadius(primary)*primary->m_orbMin*fixed(1,10));

		discDensity = rand.Fixed() * get_disc_density(primary, discMin, discMax, fixed(1,500));
	}

	//fixed discDensity = 20*rand.NFixed(4);

	//Output("Around %s: Range %f -> %f AU\n", primary->GetName().c_str(), discMin.ToDouble(), discMax.ToDouble());

	fixed initialJump = rand.NFixed(5);
	fixed pos = (fixed(1,1) - initialJump)*discMin + (initialJump*discMax);

	while (pos < discMax) {
		// periapsis, apoapsis = closest, farthest distance in orbit
		fixed periapsis = pos + pos*fixed(1,2)*rand.NFixed(2);/* + jump */;
		fixed ecc = rand.NFixed(3);
		fixed semiMajorAxis = periapsis / (fixed(1,1) - ecc);
		fixed apoapsis = 2*semiMajorAxis - periapsis;
		if (apoapsis > discMax) break;

		fixed mass;
		{
			const fixed a = pos;
			const fixed b = fixed(135,100)*apoapsis;
			mass = mass_from_disk_area(a, b, discMax);
			mass *= rand.Fixed() * discDensity;
		}
		if (mass < 0) {// hack around overflow
			Output("WARNING: planetary mass has overflowed! (child of %s)\n", primary->GetName().c_str());
			mass = fixed(Sint64(0x7fFFffFFffFFffFFull));
		}
		assert(mass >= 0);

		SystemBody *planet = system->NewBody();
		planet->m_eccentricity = ecc;
		planet->m_axialTilt = fixed(100,157)*rand.NFixed(2);
		planet->m_semiMajorAxis = semiMajorAxis;
		planet->m_type = SystemBody::TYPE_PLANET_TERRESTRIAL;
		planet->m_seed = rand.Int32();
		planet->m_parent = primary;
		planet->m_mass = mass;
		planet->m_rotationPeriod = fixed(rand.Int32(1,200), 24);

		const double e = ecc.ToDouble();

		if(primary->m_type == SystemBody::TYPE_GRAVPOINT)
			planet->m_orbit.SetShapeAroundBarycentre(semiMajorAxis.ToDouble() * AU, primary->GetMass(), planet->GetMass(), e);
		else
			planet->m_orbit.SetShapeAroundPrimary(semiMajorAxis.ToDouble() * AU, primary->GetMass(), e);

		double r1 = rand.Double(2*M_PI);		// function parameter evaluation order is implementation-dependent
		double r2 = rand.NDouble(5);			// can't put two rands in the same expression
		planet->m_orbit.SetPlane(matrix3x3d::RotateY(r1) * matrix3x3d::RotateX(-0.5*M_PI + r2*M_PI/2.0));

		planet->m_inclination = FIXED_PI;
		planet->m_inclination *= r2/2.0;
		planet->m_orbMin = periapsis;
		planet->m_orbMax = apoapsis;
		primary->m_children.push_back(planet);

		/* minimum separation between planets of 1.35 */
		pos = apoapsis * fixed(135,100);
	}

	int idx=0;
	bool make_moons = superType <= SystemBody::SUPERTYPE_STAR;

	for (std::vector<SystemBody*>::iterator i = primary->m_children.begin(); i != primary->m_children.end(); ++i) {
		// planets around a binary pair [gravpoint] -- ignore the stars...
		if ((*i)->GetSuperType() == SystemBody::SUPERTYPE_STAR) continue;
		// Turn them into something!!!!!!!
		char buf[8];
		if (superType <= SystemBody::SUPERTYPE_STAR) {
			// planet naming scheme
			snprintf(buf, sizeof(buf), " %c", 'a'+idx);
		} else {
			// moon naming scheme
			snprintf(buf, sizeof(buf), " %d", 1+idx);
		}
		(*i)->m_name = primary->GetName()+buf;
		PickPlanetType(*i, rand);
		if (make_moons) MakePlanetsAround(system, *i, rand);
		idx++;
	}
}

void StarSystemRandomGenerator::MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type, Random &rand)
{
	PROFILE_SCOPED()
	sbody->m_type = type;
	sbody->m_seed = rand.Int32();
	sbody->m_radius = fixed(rand.Int32(starTypeInfo[type].radius[0], starTypeInfo[type].radius[1]), 100);

	// Assign aspect ratios caused by equatorial bulges due to rotation. See terrain code for details.
	// XXX to do: determine aspect ratio distributions for dimmer stars. Make aspect ratios consistent with rotation speeds/stability restrictions.
	switch (type) {
		// Assign aspect ratios (roughly) between 1.0 to 1.8 with a bias towards 1 for bright stars F, A, B ,O

		// "A large fraction of hot stars are rapid rotators with surface rotational velocities
		// of more than 100 km/s (6, 7). ." Imaging the Surface of Altair, John D. Monnier, et. al. 2007
		// A reasonable amount of lot of stars will be assigned high aspect ratios.

		// Bright stars whose equatorial to polar radius ratio (the aspect ratio) is known
		// seem to tend to have values between 1.0 and around 1.5 (brief survey).
		// The limiting factor preventing much higher values seems to be stability as they
		// are rotating 80-95% of their breakup velocity.
		case SystemBody::TYPE_STAR_F:
		case SystemBody::TYPE_STAR_F_GIANT:
		case SystemBody::TYPE_STAR_F_HYPER_GIANT:
		case SystemBody::TYPE_STAR_F_SUPER_GIANT:
		case SystemBody::TYPE_STAR_A:
		case SystemBody::TYPE_STAR_A_GIANT:
		case SystemBody::TYPE_STAR_A_HYPER_GIANT:
		case SystemBody::TYPE_STAR_A_SUPER_GIANT:
		case SystemBody::TYPE_STAR_B:
		case SystemBody::TYPE_STAR_B_GIANT:
		case SystemBody::TYPE_STAR_B_SUPER_GIANT:
		case SystemBody::TYPE_STAR_B_WF:
		case SystemBody::TYPE_STAR_O:
		case SystemBody::TYPE_STAR_O_GIANT:
		case SystemBody::TYPE_STAR_O_HYPER_GIANT:
		case SystemBody::TYPE_STAR_O_SUPER_GIANT:
		case SystemBody::TYPE_STAR_O_WF: {
			fixed rnd = rand.Fixed();
			sbody->m_aspectRatio = fixed(1, 1)+fixed(8, 10)*rnd*rnd;
			break;
		}
		// aspect ratio is initialised to 1.0 for other stars currently
		default:
			break;
	}
	sbody->m_mass = fixed(rand.Int32(starTypeInfo[type].mass[0], starTypeInfo[type].mass[1]), 100);
	sbody->m_averageTemp = rand.Int32(starTypeInfo[type].tempMin, starTypeInfo[type].tempMax);
}

void StarSystemRandomGenerator::MakeRandomStar(SystemBody *sbody, Random &rand)
{
	PROFILE_SCOPED()
	SystemBody::BodyType type = SystemBody::BodyType(rand.Int32(SystemBody::TYPE_STAR_MIN, SystemBody::TYPE_STAR_MAX));
	MakeStarOfType(sbody, type, rand);
}

void StarSystemRandomGenerator::MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass, Random &rand)
{
	PROFILE_SCOPED()
	int tries = 16;
	do {
		MakeStarOfType(sbody, type, rand);
	} while ((sbody->GetMassAsFixed() > maxMass) && (--tries));
}

void StarSystemRandomGenerator::MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist, Random &rand)
{
	PROFILE_SCOPED()
	fixed m = a->GetMassAsFixed() + b->GetMassAsFixed();
	fixed a0 = b->GetMassAsFixed() / m;
	fixed a1 = a->GetMassAsFixed() / m;
	a->m_eccentricity = rand.NFixed(3);
	int mul = 1;

	do {
		switch (rand.Int32(3)) {
			case 2: a->m_semiMajorAxis = fixed(rand.Int32(100,10000), 100); break;
			case 1: a->m_semiMajorAxis = fixed(rand.Int32(10,1000), 100); break;
			default:
			case 0: a->m_semiMajorAxis = fixed(rand.Int32(1,100), 100); break;
		}
		a->m_semiMajorAxis *= mul;
		mul *= 2;
	} while (a->m_semiMajorAxis - a->m_eccentricity*a->m_semiMajorAxis < minDist);

	const double total_mass = a->GetMass() + b->GetMass();
	const double e = a->m_eccentricity.ToDouble();

	a->m_orbit.SetShapeAroundBarycentre(AU * (a->m_semiMajorAxis * a0).ToDouble(), total_mass, a->GetMass(), e);
	b->m_orbit.SetShapeAroundBarycentre(AU * (a->m_semiMajorAxis * a1).ToDouble(), total_mass, b->GetMass(), e);

	const float rotX = -0.5f*float(M_PI);//(float)(rand.Double()*M_PI/2.0);
	const float rotY = static_cast<float>(rand.Double(M_PI));
	a->m_orbit.SetPlane(matrix3x3d::RotateY(rotY) * matrix3x3d::RotateX(rotX));
	b->m_orbit.SetPlane(matrix3x3d::RotateY(rotY-M_PI) * matrix3x3d::RotateX(rotX));

	// store orbit parameters for later use to be accesible in other way than by rotMatrix
	b->m_orbitalPhaseAtStart = b->m_orbitalPhaseAtStart + FIXED_PI;
	b->m_orbitalPhaseAtStart = b->m_orbitalPhaseAtStart > 2*FIXED_PI ? b->m_orbitalPhaseAtStart - 2*FIXED_PI : b->m_orbitalPhaseAtStart;
	a->m_orbitalPhaseAtStart = a->m_orbitalPhaseAtStart > 2*FIXED_PI ? a->m_orbitalPhaseAtStart - 2*FIXED_PI : a->m_orbitalPhaseAtStart;
	a->m_orbitalPhaseAtStart = a->m_orbitalPhaseAtStart < 0 ? a->m_orbitalPhaseAtStart + 2*FIXED_PI : a->m_orbitalPhaseAtStart;
	b->m_orbitalOffset = fixed(int(round(rotY*10000)),10000);
	a->m_orbitalOffset = fixed(int(round(rotY*10000)),10000);

	fixed orbMin = a->m_semiMajorAxis - a->m_eccentricity*a->m_semiMajorAxis;
	fixed orbMax = 2*a->m_semiMajorAxis - orbMin;
	a->m_orbMin = orbMin;
	b->m_orbMin = orbMin;
	a->m_orbMax = orbMax;
	b->m_orbMax = orbMax;
}

bool StarSystemRandomGenerator::Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system, GalaxyGenerator::StarSystemConfig* config)
{
	PROFILE_SCOPED()
	RefCountedPtr<const Sector> sec = Pi::GetGalaxy()->GetSector(system->GetPath());
	const Sector::System& secSys = sec->m_systems[system->GetPath().systemIndex];

	if (config->isCustomOnly)
		return true;

	SystemBody *star[4];
	SystemBody *centGrav1(0), *centGrav2(0);

	const int numStars = secSys.GetNumStars();
	assert((numStars >= 1) && (numStars <= 4));
	if (numStars == 1) {
		SystemBody::BodyType type = sec->m_systems[system->GetPath().systemIndex].GetStarType(0);
		star[0] = system->NewBody();
		star[0]->m_parent = 0;
		star[0]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName();
		star[0]->m_orbMin = fixed();
		star[0]->m_orbMax = fixed();

		MakeStarOfType(star[0], type, rng);
		system->SetRootBody(star[0]);
		system->SetNumStars(1);
	} else {
		centGrav1 = system->NewBody();
		centGrav1->m_type = SystemBody::TYPE_GRAVPOINT;
		centGrav1->m_parent = 0;
		centGrav1->m_name = sec->m_systems[system->GetPath().systemIndex].GetName()+" A,B";
		system->SetRootBody(centGrav1);

		SystemBody::BodyType type = sec->m_systems[system->GetPath().systemIndex].GetStarType(0);
		star[0] = system->NewBody();
		star[0]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName()+" A";
		star[0]->m_parent = centGrav1;
		MakeStarOfType(star[0], type, rng);

		star[1] = system->NewBody();
		star[1]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName()+" B";
		star[1]->m_parent = centGrav1;
		MakeStarOfTypeLighterThan(star[1], sec->m_systems[system->GetPath().systemIndex].GetStarType(1), star[0]->GetMassAsFixed(), rng);

		centGrav1->m_mass = star[0]->GetMassAsFixed() + star[1]->GetMassAsFixed();
		centGrav1->m_children.push_back(star[0]);
		centGrav1->m_children.push_back(star[1]);
		// Separate stars by 0.2 radii for each, so that their planets don't bump into the other star
		const fixed minDist1 = (fixed(12,10) * star[0]->GetRadiusAsFixed() + fixed(12,10) * star[1]->GetRadiusAsFixed()) * AU_SOL_RADIUS;
try_that_again_guvnah:
		MakeBinaryPair(star[0], star[1], minDist1, rng);

		system->SetNumStars(2);

		if (numStars > 2) {
			if (star[0]->m_orbMax > fixed(100,1)) {
				// reduce to < 100 AU...
				goto try_that_again_guvnah;
			}
			// 3rd and maybe 4th star
			if (numStars == 3) {
				star[2] = system->NewBody();
				star[2]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName()+" C";
				star[2]->m_orbMin = 0;
				star[2]->m_orbMax = 0;
				MakeStarOfTypeLighterThan(star[2], sec->m_systems[system->GetPath().systemIndex].GetStarType(2), star[0]->GetMassAsFixed(), rng);
				centGrav2 = star[2];
				system->SetNumStars(3);
			} else {
				centGrav2 = system->NewBody();
				centGrav2->m_type = SystemBody::TYPE_GRAVPOINT;
				centGrav2->m_name = sec->m_systems[system->GetPath().systemIndex].GetName()+" C,D";
				centGrav2->m_orbMax = 0;

				star[2] = system->NewBody();
				star[2]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName()+" C";
				star[2]->m_parent = centGrav2;
				MakeStarOfTypeLighterThan(star[2], sec->m_systems[system->GetPath().systemIndex].GetStarType(2), star[0]->GetMassAsFixed(), rng);

				star[3] = system->NewBody();
				star[3]->m_name = sec->m_systems[system->GetPath().systemIndex].GetName()+" D";
				star[3]->m_parent = centGrav2;
				MakeStarOfTypeLighterThan(star[3], sec->m_systems[system->GetPath().systemIndex].GetStarType(3), star[2]->GetMassAsFixed(), rng);

				// Separate stars by 0.2 radii for each, so that their planets don't bump into the other star
				const fixed minDist2 = (fixed(12,10) * star[2]->GetRadiusAsFixed() + fixed(12,10) * star[3]->GetRadiusAsFixed()) * AU_SOL_RADIUS;
				MakeBinaryPair(star[2], star[3], minDist2, rng);
				centGrav2->m_mass = star[2]->GetMassAsFixed() + star[3]->GetMassAsFixed();
				centGrav2->m_children.push_back(star[2]);
				centGrav2->m_children.push_back(star[3]);
				system->SetNumStars(4);
			}
			SystemBody *superCentGrav = system->NewBody();
			superCentGrav->m_type = SystemBody::TYPE_GRAVPOINT;
			superCentGrav->m_parent = 0;
			superCentGrav->m_name = sec->m_systems[system->GetPath().systemIndex].GetName();
			centGrav1->m_parent = superCentGrav;
			centGrav2->m_parent = superCentGrav;
			system->SetRootBody(superCentGrav);
			const fixed minDistSuper = star[0]->m_orbMax + star[2]->m_orbMax;
			MakeBinaryPair(centGrav1, centGrav2, 4*minDistSuper, rng);
			superCentGrav->m_children.push_back(centGrav1);
			superCentGrav->m_children.push_back(centGrav2);

		}
	}

	// used in MakeShortDescription
	// XXX except this does not reflect the actual mining happening in this system
	system->SetMetallicity(starMetallicities[system->GetRootBody()->GetType()]);

	// store all of the stars first ...
	for (unsigned i=0; i<system->GetNumStars(); i++) {
		system->AddStar(star[i]);
	}
	// ... because we need them when making planets to calculate surface temperatures
	for (auto s : system->GetStars()) {
		MakePlanetsAround(system, s, rng);
	}

	if (system->GetNumStars() > 1)
		MakePlanetsAround(system, centGrav1, rng);
	if (system->GetNumStars() == 4)
		MakePlanetsAround(system, centGrav2, rng);

	// an example export of generated system, can be removed during the merge
	//char filename[500];
	//snprintf(filename, 500, "tmp-sys/%s.lua", GetName().c_str());
	//ExportToLua(filename);

#ifdef DEBUG_DUMP
	Dump();
#endif /* DEBUG_DUMP */
	return true;
}

/*
 * Position a surface starport anywhere. Space.cpp::MakeFrameFor() ensures it
 * is on dry land (discarding this position if necessary)
 */
void PopulateStarSystemGenerator::PositionSettlementOnPlanet(SystemBody* sbody)
{
	PROFILE_SCOPED()
	Random r(sbody->GetSeed());
	// used for orientation on planet surface
	double r2 = r.Double(); 	// function parameter evaluation order is implementation-dependent
	double r1 = r.Double();		// can't put two rands in the same expression
	sbody->m_orbit.SetPlane(matrix3x3d::RotateZ(2*M_PI*r1) * matrix3x3d::RotateY(2*M_PI*r2));

	// store latitude and longitude to equivalent orbital parameters to
	// be accessible easier
	sbody->m_inclination = fixed(r1*10000,10000) + FIXED_PI/2;	// latitide
	sbody->m_orbitalOffset = FIXED_PI/2;						// longitude

}

/*
 * Set natural resources, tech level, industry strengths and population levels
 */
void PopulateStarSystemGenerator::PopulateStage1(SystemBody* sbody, StarSystem::GeneratorAPI *system, fixed &outTotalPop)
{
	PROFILE_SCOPED()
	for (auto child : sbody->GetChildren()) {
		PopulateStage1(child, system, outTotalPop);
	}

	// unexplored systems have no population (that we know about)
	if (system->GetUnexplored()) {
		sbody->m_population = outTotalPop = fixed();
		return;
	}

	// grav-points have no population themselves
	if (sbody->GetType() == SystemBody::TYPE_GRAVPOINT) {
		sbody->m_population = fixed();
		return;
	}

	Uint32 _init[6] = { system->GetPath().systemIndex, Uint32(system->GetPath().sectorX),
			Uint32(system->GetPath().sectorY), Uint32(system->GetPath().sectorZ), UNIVERSE_SEED, Uint32(sbody->GetSeed()) };

	Random rand;
	rand.seed(_init, 6);

	RefCountedPtr<Random> namerand(new Random);
	namerand->seed(_init, 6);

	sbody->m_population = fixed();

	/* Bad type of planet for settlement */
	if ((sbody->GetAverageTemp() > CELSIUS+100) || (sbody->GetAverageTemp() < 100) ||
	    (sbody->GetType() != SystemBody::TYPE_PLANET_TERRESTRIAL && sbody->GetType() != SystemBody::TYPE_PLANET_ASTEROID)) {

        // orbital starports should carry a small amount of population
        if (sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL) {
			sbody->m_population = fixed(1,100000);
			outTotalPop += sbody->m_population;
        }

		return;
	}

	sbody->m_agricultural = fixed();

	if (sbody->GetLifeAsFixed() > fixed(9,10)) {
		sbody->m_agricultural = Clamp(fixed(1,1) - fixed(CELSIUS+25-sbody->GetAverageTemp(), 40), fixed(), fixed(1,1));
		system->SetAgricultural(system->GetAgricultural() + 2*sbody->m_agricultural);
	} else if (sbody->GetLifeAsFixed() > fixed(1,2)) {
		sbody->m_agricultural = Clamp(fixed(1,1) - fixed(CELSIUS+30-sbody->GetAverageTemp(), 50), fixed(), fixed(1,1));
		system->SetAgricultural(system->GetAgricultural() + 1*sbody->m_agricultural);
	} else {
		// don't bother populating crap planets
		if (sbody->GetMetallicityAsFixed() < fixed(5,10) &&
			sbody->GetMetallicityAsFixed() < (fixed(1,1) - system->GetHumanProx())) return;
	}

	const int NUM_CONSUMABLES = 10;
	const GalacticEconomy::Commodity consumables[NUM_CONSUMABLES] = {
		GalacticEconomy::Commodity::AIR_PROCESSORS,
		GalacticEconomy::Commodity::GRAIN,
		GalacticEconomy::Commodity::FRUIT_AND_VEG,
		GalacticEconomy::Commodity::ANIMAL_MEAT,
		GalacticEconomy::Commodity::LIQUOR,
		GalacticEconomy::Commodity::CONSUMER_GOODS,
		GalacticEconomy::Commodity::MEDICINES,
		GalacticEconomy::Commodity::HAND_WEAPONS,
		GalacticEconomy::Commodity::NARCOTICS,
		GalacticEconomy::Commodity::LIQUID_OXYGEN
	};

	/* Commodities we produce (mining and agriculture) */

	for (int i = 1; i < GalacticEconomy::COMMODITY_COUNT; i++) {
		const GalacticEconomy::CommodityInfo &info = GalacticEconomy::COMMODITY_DATA[i];

		fixed affinity = fixed(1,1);
		if (info.econType & GalacticEconomy::ECON_AGRICULTURE) {
			affinity *= 2*sbody->GetAgriculturalAsFixed();
		}
		if (info.econType & GalacticEconomy::ECON_INDUSTRY) affinity *= system->GetIndustrial();
		// make industry after we see if agriculture and mining are viable
		if (info.econType & GalacticEconomy::ECON_MINING) {
			affinity *= sbody->GetMetallicityAsFixed();
		}
		affinity *= rand.Fixed();
		// producing consumables is wise
		for (int j=0; j<NUM_CONSUMABLES; j++) {
			if (GalacticEconomy::Commodity(i) == consumables[j]) {
				affinity *= 2;
				break;
			}
		}
		assert(affinity >= 0);
		/* workforce... */
		sbody->m_population += affinity * system->GetHumanProx();

		int howmuch = (affinity * 256).ToInt32();

		system->AddTradeLevel(GalacticEconomy::Commodity(i), -2*howmuch);
		for (int j=0; j < GalacticEconomy::CommodityInfo::MAX_ECON_INPUTS; ++j) {
			if (info.inputs[j] == GalacticEconomy::Commodity::NONE) continue;
			system->AddTradeLevel(GalacticEconomy::Commodity(info.inputs[j]), howmuch);
		}
	}

	if (!system->HasCustomBodies() && sbody->GetPopulationAsFixed() > 0)
		sbody->m_name = Pi::luaNameGen->BodyName(sbody, namerand);

	// Add a bunch of things people consume
	for (int i=0; i<NUM_CONSUMABLES; i++) {
		GalacticEconomy::Commodity t = consumables[i];
		if (sbody->GetLifeAsFixed() > fixed(1,2)) {
			// life planets can make this jizz probably
			if ((t == GalacticEconomy::Commodity::AIR_PROCESSORS) ||
			    (t == GalacticEconomy::Commodity::LIQUID_OXYGEN) ||
			    (t == GalacticEconomy::Commodity::GRAIN) ||
			    (t == GalacticEconomy::Commodity::FRUIT_AND_VEG) ||
			    (t == GalacticEconomy::Commodity::ANIMAL_MEAT)) {
				continue;
			}
		}
		system->AddTradeLevel(GalacticEconomy::Commodity(t), rand.Int32(32,128));
	}
	// well, outdoor worlds should have way more people
	sbody->m_population = fixed(1,10)*sbody->m_population + sbody->m_population*sbody->GetAgriculturalAsFixed();

//	Output("%s: pop %.3f billion\n", name.c_str(), sbody->m_population.ToFloat());

	outTotalPop += sbody->GetPopulationAsFixed();
}

static bool check_unique_station_name(const std::string & name, const StarSystem * system) {
	PROFILE_SCOPED()
	bool ret = true;
	for (const SystemBody *station : system->GetSpaceStations())
		if (station->GetName() == name) {
			ret = false;
			break;
		}
	return ret;
}

static std::string gen_unique_station_name(SystemBody *sp, const StarSystem *system, RefCountedPtr<Random> &namerand) {
	PROFILE_SCOPED()
	std::string name;
	do {
		name = Pi::luaNameGen->BodyName(sp, namerand);
	} while (!check_unique_station_name(name, system));
	return name;
}

void PopulateStarSystemGenerator::PopulateAddStations(SystemBody* sbody, StarSystem::GeneratorAPI *system)
{
	PROFILE_SCOPED()
	for (auto child : sbody->GetChildren())
		PopulateAddStations(child, system);

	Uint32 _init[6] = { system->GetPath().systemIndex, Uint32(system->GetPath().sectorX),
			Uint32(system->GetPath().sectorY), Uint32(system->GetPath().sectorZ), sbody->GetSeed(), UNIVERSE_SEED };

	Random rand;
	rand.seed(_init, 6);

	RefCountedPtr<Random> namerand(new Random);
	namerand->seed(_init, 6);

	if (sbody->GetPopulationAsFixed() < fixed(1,1000)) return;

	fixed orbMaxS = fixed(1,4)*CalcHillRadius(sbody);
	fixed orbMinS = 4 * sbody->GetRadiusAsFixed() * AU_EARTH_RADIUS;
	if (sbody->GetNumChildren()) orbMaxS = std::min(orbMaxS, fixed(1,2) * sbody->GetChildren()[0]->GetOrbMinAsFixed());

	// starports - orbital
	fixed pop = sbody->GetPopulationAsFixed() + rand.Fixed();
	if( orbMinS < orbMaxS )
	{
		pop -= rand.Fixed();
		Uint32 NumToMake = 0;
		while(pop >= 0) {
			++NumToMake;
			pop -= rand.Fixed();
		}
		for( Uint32 i=0; i<NumToMake; i++ ) {
			SystemBody *sp = system->NewBody();
			sp->m_type = SystemBody::TYPE_STARPORT_ORBITAL;
			sp->m_seed = rand.Int32();
			sp->m_parent = sbody;
			sp->m_rotationPeriod = fixed(1,3600);
			sp->m_averageTemp = sbody->GetAverageTemp();
			sp->m_mass = 0;

			// place stations between min and max orbits to reduce the number of extremely close/fast orbits
			sp->m_semiMajorAxis = orbMinS + ((orbMaxS - orbMinS) / 4);
			sp->m_eccentricity = fixed();
			sp->m_axialTilt = fixed();

			sp->m_orbit.SetShapeAroundPrimary(sp->GetSemiMajorAxisAsFixed().ToDouble()*AU, sbody->GetMassAsFixed().ToDouble() * EARTH_MASS, 0.0);
			if (NumToMake > 1) {
				sp->m_orbit.SetPlane(matrix3x3d::RotateZ(double(i) * (M_PI / double(NumToMake-1))));
			} else {
				sp->m_orbit.SetPlane(matrix3x3d::Identity());
			}

			sp->m_inclination = fixed();
			sbody->m_children.insert(sbody->m_children.begin(), sp);
			system->AddSpaceStation(sp);
			sp->m_orbMin = sp->GetSemiMajorAxisAsFixed();
			sp->m_orbMax = sp->GetSemiMajorAxisAsFixed();

			sp->m_name = gen_unique_station_name(sp, system, namerand);
		}
	}
	// starports - surface
	// give it a fighting chance of having a decent number of starports (*3)
	pop = sbody->GetPopulationAsFixed() + (rand.Fixed() * 3);
	int max = 6;
	while (max-- > 0) {
		pop -= rand.Fixed();
		if (pop < 0) break;

		SystemBody *sp = system->NewBody();
		sp->m_type = SystemBody::TYPE_STARPORT_SURFACE;
		sp->m_seed = rand.Int32();
		sp->m_parent = sbody;
		sp->m_averageTemp = sbody->GetAverageTemp();
		sp->m_mass = 0;
		sp->m_name = gen_unique_station_name(sp, system, namerand);
		memset(&sp->m_orbit, 0, sizeof(Orbit));
		PositionSettlementOnPlanet(sp);
		sbody->m_children.insert(sbody->m_children.begin(), sp);
		system->AddSpaceStation(sp);
	}

	// garuantee that there is always a star port on a populated world
	if( !system->HasSpaceStations() )
	{
		SystemBody *sp = system->NewBody();
		sp->m_type = SystemBody::TYPE_STARPORT_SURFACE;
		sp->m_seed = rand.Int32();
		sp->m_parent = sbody;
		sp->m_averageTemp = sbody->m_averageTemp;
		sp->m_mass = 0;
		sp->m_name = gen_unique_station_name(sp, system, namerand);
		memset(&sp->m_orbit, 0, sizeof(Orbit));
		PositionSettlementOnPlanet(sp);
		sbody->m_children.insert(sbody->m_children.begin(), sp);
		system->AddSpaceStation(sp);
	}
}

void PopulateStarSystemGenerator::MakeShortDescription(RefCountedPtr<StarSystem::GeneratorAPI> system, Random &rand)
{
	PROFILE_SCOPED()
	if ((system->GetIndustrial() > system->GetMetallicity()) && (system->GetIndustrial() > system->GetAgricultural())) {
		system->SetEconType(GalacticEconomy::ECON_INDUSTRY);
	} else if (system->GetMetallicity() > system->GetAgricultural()) {
		system->SetEconType(GalacticEconomy::ECON_MINING);
	} else {
		system->SetEconType(GalacticEconomy::ECON_AGRICULTURE);
	}

	if (system->GetUnexplored()) {
		system->SetShortDesc(Lang::UNEXPLORED_SYSTEM_NO_DATA);
	}

	/* Total population is in billions */
	else if(system->GetTotalPop() == 0) {
		system->SetShortDesc(Lang::SMALL_SCALE_PROSPECTING_NO_SETTLEMENTS);
	} else if (system->GetTotalPop() < fixed(1,10)) {
		switch (system->GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: system->SetShortDesc(Lang::SMALL_INDUSTRIAL_OUTPOST); break;
			case GalacticEconomy::ECON_MINING: system->SetShortDesc(Lang::SOME_ESTABLISHED_MINING); break;
			case GalacticEconomy::ECON_AGRICULTURE: system->SetShortDesc(Lang::YOUNG_FARMING_COLONY); break;
		}
	} else if (system->GetTotalPop() < fixed(1,2)) {
		switch (system->GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: system->SetShortDesc(Lang::INDUSTRIAL_COLONY); break;
			case GalacticEconomy::ECON_MINING: system->SetShortDesc(Lang::MINING_COLONY); break;
			case GalacticEconomy::ECON_AGRICULTURE: system->SetShortDesc(Lang::OUTDOOR_AGRICULTURAL_WORLD); break;
		}
	} else if (system->GetTotalPop() < fixed(5,1)) {
		switch (system->GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: system->SetShortDesc(Lang::HEAVY_INDUSTRY); break;
			case GalacticEconomy::ECON_MINING: system->SetShortDesc(Lang::EXTENSIVE_MINING); break;
			case GalacticEconomy::ECON_AGRICULTURE: system->SetShortDesc(Lang::THRIVING_OUTDOOR_WORLD); break;
		}
	} else {
		switch (system->GetEconType()) {
			case GalacticEconomy::ECON_INDUSTRY: system->SetShortDesc(Lang::INDUSTRIAL_HUB_SYSTEM); break;
			case GalacticEconomy::ECON_MINING: system->SetShortDesc(Lang::VAST_STRIP_MINE); break;
			case GalacticEconomy::ECON_AGRICULTURE: system->SetShortDesc(Lang::HIGH_POPULATION_OUTDOOR_WORLD); break;
		}
	}
}

/* percent */
static const int MAX_COMMODITY_BASE_PRICE_ADJUSTMENT = 25;

bool PopulateStarSystemGenerator::Apply(Random& rng, RefCountedPtr<StarSystem::GeneratorAPI> system,
		GalaxyGenerator::StarSystemConfig* config)
{
	PROFILE_SCOPED()
	const bool addSpaceStations = !config->isCustomOnly;
	Uint32 _init[5] = { system->GetPath().systemIndex, Uint32(system->GetPath().sectorX), Uint32(system->GetPath().sectorY), Uint32(system->GetPath().sectorZ), UNIVERSE_SEED };
	Random rand;
	rand.seed(_init, 5);

	/* Various system-wide characteristics */
	// This is 1 in sector (0,0,0) and approaches 0 farther out
	// (1,0,0) ~ .688, (1,1,0) ~ .557, (1,1,1) ~ .48
	system->SetHumanProx(Pi::GetGalaxy()->GetFactions()->IsHomeSystem(system->GetPath()) ? fixed(2,3): fixed(3,1) /
		isqrt(9 + 10*(system->GetPath().sectorX*system->GetPath().sectorX + system->GetPath().sectorY*system->GetPath().sectorY + system->GetPath().sectorZ*system->GetPath().sectorZ)));
	system->SetEconType(GalacticEconomy::ECON_INDUSTRY);
	system->SetIndustrial(rand.Fixed());
	system->SetAgricultural(0);

	/* system attributes */
	fixed totalPop = fixed();
	PopulateStage1(system->GetRootBody().Get(), system.Get(), totalPop);
	system->SetTotalPop(totalPop);

//	Output("Trading rates:\n");
	// So now we have balances of trade of various commodities.
	// Lets use black magic to turn these into percentage base price
	// alterations
	int maximum = 0;
	for (int i = 1; i < GalacticEconomy::COMMODITY_COUNT; i++) {
		maximum = std::max(abs(system->GetTradeLevel()[i]), maximum);
	}
	if (maximum) for (int i = 1; i < GalacticEconomy::COMMODITY_COUNT; i++) {
		system->SetTradeLevel(GalacticEconomy::Commodity(i), (system->GetTradeLevel()[i] * MAX_COMMODITY_BASE_PRICE_ADJUSTMENT) / maximum);
		system->AddTradeLevel(GalacticEconomy::Commodity(i), rand.Int32(-5, 5));
	}

// Unused?
//	for (int i=(int)Equip::FIRST_COMMODITY; i<=(int)Equip::LAST_COMMODITY; i++) {
//		Equip::Type t = (Equip::Type)i;
//		const EquipType &type = Equip::types[t];
//		Output("%s: %d%%\n", type.name, m_tradeLevel[t]);
//	}
//	Output("System total population %.3f billion\n", m_totalPop.ToFloat());
	SysPolit sysPolit;
	Polit::GetSysPolitStarSystem(system.Get(), system->GetTotalPop(), sysPolit);
	system->SetSysPolit(sysPolit);

	if (addSpaceStations) {
		PopulateAddStations(system->GetRootBody().Get(), system.Get());
	}

	if (!system->GetShortDescription().size())
		MakeShortDescription(system, rand);

	return true;
}
