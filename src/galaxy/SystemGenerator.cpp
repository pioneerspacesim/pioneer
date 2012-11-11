// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemGenerator.h"
#include "Sector.h"
#include "CustomSystem.h"

namespace SystemGeneration
{

//-----------------------------------------------------------------------------
// Tables & Constants

	// very crudely
	static const fixed AU_SOL_RADIUS   = fixed(305,65536);
	static const fixed AU_EARTH_RADIUS = fixed(3, 65536);

	static const struct StarTypeInfo {
		SystemBody::BodySuperType supertype;
		int mass[2]; // min,max % sol for stars, unused for planets
		int radius[2]; // min,max % sol radii for stars, % earth radii for planets
		int tempMin, tempMax;
	} starTypeInfo[] = {
		{
			SystemBody::SUPERTYPE_NONE, {}, {},
			0, 0
		}, {
			SystemBody::SUPERTYPE_STAR, //Brown Dwarf
			{2,8}, {10,30},
			1000, 2000
		}, {
			SystemBody::SUPERTYPE_STAR,  //white dwarf
			{20,100}, {1,2},
			4000, 40000
		}, {
			SystemBody::SUPERTYPE_STAR, //M
			{10,47}, {30,60},
			2000, 3500
		}, {
			SystemBody::SUPERTYPE_STAR, //K
			{50,78}, {60,100},
			3500, 5000
		}, {
			SystemBody::SUPERTYPE_STAR, //G
			{80,110}, {80,120},
			5000, 6000
		}, {
			SystemBody::SUPERTYPE_STAR, //F
			{115,170}, {110,150},
			6000, 7500
		}, {
			SystemBody::SUPERTYPE_STAR, //A
			{180,320}, {120,220},
			7500, 10000
		}, {
			SystemBody::SUPERTYPE_STAR,  //B
			{200,300}, {120,290},
			10000, 30000
		}, {
			SystemBody::SUPERTYPE_STAR, //O
			{300,400}, {200,310},
			30000, 60000
		}, {
			SystemBody::SUPERTYPE_STAR, //M Giant
			{60,357}, {2000,5000},
			2500, 3500
		}, {
			SystemBody::SUPERTYPE_STAR, //K Giant
			{125,500}, {1500,3000},
			3500, 5000
		}, {
			SystemBody::SUPERTYPE_STAR, //G Giant
			{200,800}, {1000,2000},
			5000, 6000
		}, {
			SystemBody::SUPERTYPE_STAR, //F Giant
			{250,900}, {800,1500},
			6000, 7500
		}, {
			SystemBody::SUPERTYPE_STAR, //A Giant
			{400,1000}, {600,1000},
			7500, 10000
		}, {
			SystemBody::SUPERTYPE_STAR,  //B Giant
			{500,1000}, {600,1000},
			10000, 30000
		}, {
			SystemBody::SUPERTYPE_STAR, //O Giant
			{600,1200}, {600,1000},
			30000, 60000
		}, {
			SystemBody::SUPERTYPE_STAR, //M Super Giant
			{1050,5000}, {7000,15000},
			2500, 3500
		}, {
			SystemBody::SUPERTYPE_STAR, //K Super Giant
			{1100,5000}, {5000,9000},
			3500, 5000
		}, {
			SystemBody::SUPERTYPE_STAR, //G Super Giant
			{1200,5000}, {4000,8000},
			5000, 6000
		}, {
			SystemBody::SUPERTYPE_STAR, //F Super Giant
			{1500,6000}, {3500,7000},
			6000, 7500
		}, {
			SystemBody::SUPERTYPE_STAR, //A Super Giant
			{2000,8000}, {3000,6000},
			7500, 10000
		}, {
			SystemBody::SUPERTYPE_STAR,  //B Super Giant
			{3000,9000}, {2500,5000},
			10000, 30000
		}, {
			SystemBody::SUPERTYPE_STAR, //O Super Giant
			{5000,10000}, {2000,4000},
			30000, 60000
		}, {
			SystemBody::SUPERTYPE_STAR, //M Hyper Giant
			{5000,15000}, {20000,40000},
			2500, 3500
		}, {
			SystemBody::SUPERTYPE_STAR, //K Hyper Giant
			{5000,17000}, {17000,25000},
			3500, 5000
		}, {
			SystemBody::SUPERTYPE_STAR, //G Hyper Giant
			{5000,18000}, {14000,20000},
			5000, 6000
		}, {
			SystemBody::SUPERTYPE_STAR, //F Hyper Giant
			{5000,19000}, {12000,17500},
			6000, 7500
		}, {
			SystemBody::SUPERTYPE_STAR, //A Hyper Giant
			{5000,20000}, {10000,15000},
			7500, 10000
		}, {
			SystemBody::SUPERTYPE_STAR,  //B Hyper Giant
			{5000,23000}, {6000,10000},
			10000, 30000
		}, {
			SystemBody::SUPERTYPE_STAR, //O Hyper Giant
			{10000,30000}, {4000,7000},
			30000, 60000
		}, {
			SystemBody::SUPERTYPE_STAR,  // M WF
			{2000,5000}, {2500,5000},
			25000, 35000
		}, {
			SystemBody::SUPERTYPE_STAR,  // B WF
			{2000,7500}, {2500,5000},
			35000, 45000
		}, {
			SystemBody::SUPERTYPE_STAR,  // O WF
			{2000,10000}, {2500,5000},
			45000, 60000
		}, {
			SystemBody::SUPERTYPE_STAR,  // S BH
			{20,2000}, {0,0},	// XXX black holes are < 1 Sol radii big; this is clamped to a non-zero value later
			10, 24
		}, {
			SystemBody::SUPERTYPE_STAR,  // IM BH
			{900000,1000000}, {100,500},
			1, 10
		}, {
			SystemBody::SUPERTYPE_STAR,  // SM BH
			{2000000,5000000}, {10000,20000},
			10, 24
		}
	/*	}, {
			SystemBody::SUPERTYPE_GAS_GIANT,
			{}, 950, Lang::MEDIUM_GAS_GIANT,
		}, {
			SystemBody::SUPERTYPE_GAS_GIANT,
			{}, 1110, Lang::LARGE_GAS_GIANT,
		}, {
			SystemBody::SUPERTYPE_GAS_GIANT,
			{}, 1500, Lang::VERY_LARGE_GAS_GIANT,
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 1, Lang::ASTEROID,
			"icons/object_planet_asteroid.png"
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 2, "Large asteroid",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 26, "Small, rocky dwarf planet", // moon radius
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 26, "Small, rocky dwarf planet", // dwarf2 for moon-like colours
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 52, "Small, rocky planet with a thin atmosphere", // mars radius
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Rocky frozen planet with a thin nitrogen atmosphere", // earth radius
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Dead world that once housed it's own intricate ecosystem.", // earth radius
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Rocky planet with a carbon dioxide atmosphere",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Rocky planet with a methane atmosphere",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Water world with vast oceans and a thick nitrogen atmosphere",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Rocky planet with a thick carbon dioxide atmosphere",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Rocky planet with a thick methane atmosphere",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "Highly volcanic world",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 100, "World with indigenous life and an oxygen atmosphere",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 60, "Marginal terraformed world with minimal plant life",
		}, {
			SystemBody::SUPERTYPE_ROCKY_PLANET,
			{}, 90, "Fully terraformed world with introduced species from numerous successful colonies",
		}, {
			SystemBody::SUPERTYPE_STARPORT,
			{}, 0, Lang::ORBITAL_STARPORT,
		}, {
			SystemBody::SUPERTYPE_STARPORT,
			{}, 0, Lang::STARPORT,
		}*/
	};

	//-----------------------------------------------------------------------------
	// Private Classes

	class AccretionDisc {
	public:
		AccretionDisc(SystemBody* rootBody, SystemBody* primary, int numStars, MTRand& rand);

		inline const fixed AccretableMass(fixed inner, fixed outer) const;
	
		fixed min;
		fixed max;
	private:
		fixed density;

		inline const fixed MassFromDiskArea(fixed a, fixed b, fixed max) const;
		inline const fixed GetDiscDensity(SystemBody *primary, fixed discMin, fixed discMax, fixed percentOfPrimaryMass) const;
	};

//-----------------------------------------------------------------------------
// Build

	const int SystemGenerator::NumStars() const
	{ 
		if (Custom()) {
			return Custom()->numStars; 
		} else {
			return SectorSystem().numStars;
		} 
	}	

	/*
	 * 0 - ~500ly from sol: explored
	 * ~500ly - ~700ly (65-90 sectors): gradual
	 * ~700ly+: unexplored
	 */
	const bool SystemGenerator::Unexplored() 
	{
		if (Custom() && !Custom()->want_rand_explored ) {
			return !Custom()->explored;    // may be defined manually in the custom system definition
		} else {
			int dist = isqrt(1 + m_path.sectorX*m_path.sectorX + m_path.sectorY*m_path.sectorY + m_path.sectorZ*m_path.sectorZ);
			return (dist > 90) || (dist > 65 && systemRand().Int32(dist) > 40);
		}
	}

	const fixed SystemGenerator::Metallicity() const
	{ 
		return StarSystem::starMetallicities[m_rootBody->type]; 
	}

	 SystemBody* SystemGenerator::AddStarsTo(BodyList& bodies)
	{
		SystemBody *star[4];

		const int starCount = NumStars();

		// For clarity, I've separated out each case here despite there being code in common.
		switch (starCount) 
		{
			// system with a single star
			case 1: {
				m_rootBody = AddStarOfType(bodies, Name(), SectorSystem().starType[0]);
				break;
			}
			// binary system
			case 2: {
				star[0]  = AddStarOfType           (bodies, Name()+" A", SectorSystem().starType[0]);
				star[1]  = AddStarOfTypeLighterThan(bodies, Name()+" B", SectorSystem().starType[1], star[0]->mass);
				m_centGrav1 = AddGravPoint(bodies, Name()+" A,B", star[0], star[1], false);
				m_rootBody    = m_centGrav1;
				break;
			}
			// trinary system
			case 3: {
				star[0] = AddStarOfType           (bodies, Name()+" A", SectorSystem().starType[0]);
				star[1] = AddStarOfTypeLighterThan(bodies, Name()+" B", SectorSystem().starType[1], star[0]->mass);
				star[2] = AddStarOfTypeLighterThan(bodies, Name()+" C", SectorSystem().starType[2], star[0]->mass);				
				m_centGrav1 = AddGravPoint(bodies, Name()+" A,B", star[0], star[1], true);
				m_centGrav2 = star[2];
				m_rootBody    = AddGravPoint(bodies, Name(), m_centGrav1, m_centGrav2, false, (star[0]->orbMax + star[2]->orbMax) * 4);
				break;
			}
			// quarternary(?) system
			case 4: {
				star[0] = AddStarOfType           (bodies, Name()+" A", SectorSystem().starType[0]);
				star[1] = AddStarOfTypeLighterThan(bodies, Name()+" B", SectorSystem().starType[1], star[0]->mass);
				star[2] = AddStarOfTypeLighterThan(bodies, Name()+" C", SectorSystem().starType[2], star[0]->mass);
				star[3] = AddStarOfTypeLighterThan(bodies, Name()+" D", SectorSystem().starType[3], star[2]->mass);
				m_centGrav1 = AddGravPoint(bodies, Name()+" A,B", star[0], star[1], true);
				m_centGrav2 = AddGravPoint(bodies, Name()+" C,D", star[2], star[3], false);
				m_rootBody = AddGravPoint(bodies, Name(), m_centGrav1, m_centGrav2, false, (star[0]->orbMax + star[2]->orbMax) * 4);
				break;
			}
			default: assert((starCount >= 1) && (starCount <= 4)); break;
		}

		return m_rootBody;
	}


	void SystemGenerator::AddPlanetsTo(BodyList& bodies)
	{
		int bodyCount = bodies.size();
		for (int i=0; i<bodyCount; i++) {
			if (bodies[i]->type != SystemBody::TYPE_GRAVPOINT) AddPlanetsAround(bodies, bodies[i]);
		}

		if (m_centGrav1)                    AddPlanetsAround(bodies, m_centGrav1);
		if (m_centGrav2 && NumStars() == 4) AddPlanetsAround(bodies, m_centGrav2);

	}

	//-----------------------------------------------------------------------------
	// Private Build

	SystemBody* SystemGenerator::AddStarOfType(BodyList& bodies, std::string name, SystemBody::BodyType type)
	{
		SystemBody* star = NewBody(bodies, NULL, name, type);
		MakeStar(star);
		return(star);
	}

	void SystemGenerator::MakeStar(SystemBody *sbody)
	{
		MTRand &rand = systemRand();
		const SystemBody::BodyType type = sbody->type;

		sbody->seed = rand.Int32();
		sbody->radius = fixed(rand.Int32(starTypeInfo[type].radius[0], starTypeInfo[type].radius[1]), 100);
		sbody->mass   = fixed(rand.Int32(starTypeInfo[type].mass[0], starTypeInfo[type].mass[1]), 100);
		sbody->averageTemp = rand.Int32(starTypeInfo[type].tempMin, starTypeInfo[type].tempMax);
	}

	SystemBody* SystemGenerator::AddStarOfTypeLighterThan(BodyList& bodies, std::string name, SystemBody::BodyType type, fixed maxMass)
	{
		SystemBody* star = NewBody(bodies, NULL, name, type);
		int tries = 16;
		do {
			MakeStar(star);
		} while ((star->mass > maxMass) && (--tries));
		return star;
	}

	SystemBody* SystemGenerator::AddGravPoint(BodyList& bodies, std::string name, SystemBody* a, SystemBody* b, bool limitOrbit, fixed minDist)
	{
		// make the grav point
		SystemBody* gravPoint = NewBody(bodies, NULL, name, SystemBody::TYPE_GRAVPOINT);
		gravPoint-> mass = a->mass + b->mass;

		// make the two bodies children of the grav point
		gravPoint->children.push_back(a);
		gravPoint->children.push_back(b);
		a->parent = gravPoint;
		b->parent = gravPoint;

		// make the two bodies a binary pair
		MakeBinaryPair(a, b, minDist);

		// retry until orbMax value less than 100 if requested
		while (limitOrbit && a->orbMax > fixed(100,1)) 	MakeBinaryPair(a, b, minDist);

		// return
		return gravPoint;
	}


	SystemBody* SystemGenerator::AddGravPoint(BodyList& bodies, std::string name, SystemBody* a, SystemBody* b, bool limitOrbit)
	{
		return AddGravPoint(bodies, name, a, b, limitOrbit, (a->radius + b->radius) * AU_SOL_RADIUS);
	}

	void SystemGenerator::MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist)
	{
		MTRand &rand = systemRand();
	
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

		const float rotX = -0.5f*float(M_PI);//(float)(rand.Double()*M_PI/2.0);
		const float rotY = static_cast<float>(rand.Double(M_PI));
		a->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY) * matrix4x4d::RotateXMatrix(rotX);
		b->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY-M_PI) * matrix4x4d::RotateXMatrix(rotX);

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

	const fixed AccretionDisc::MassFromDiskArea(fixed a, fixed b, fixed max) const
	{
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

	const fixed AccretionDisc::GetDiscDensity(SystemBody *primary, fixed discMin, fixed discMax, fixed percentOfPrimaryMass) const
	{
		discMax = std::max(discMax, discMin);
		fixed total = MassFromDiskArea(discMin, discMax, discMax);
		return primary->GetMassInEarths() * percentOfPrimaryMass / total;
	}

	const fixed AccretionDisc::AccretableMass(fixed inner, fixed outer) const
	{
		return MassFromDiskArea(inner, outer, max) * density;
	}

	AccretionDisc::AccretionDisc(SystemBody* rootBody, SystemBody* primary, int numStars, MTRand& rand): min(fixed(0)), max(fixed(5000,1))
	{
		SystemBody::BodySuperType superType = primary->GetSuperType();

		if (superType <= SystemBody::SUPERTYPE_STAR) {
			if (primary->type == SystemBody::TYPE_GRAVPOINT) {
				/* around a binary */
				min = primary->children[0]->orbMax * ACCRETIONDISC_SAFE_BINARY_DIST;
			} else {
				/* correct thing is roche limit, but lets ignore that because
				 * it depends on body densities and gives some strange results */
				min = 4 * primary->radius * AU_SOL_RADIUS;
			}
			if (primary->type == SystemBody::TYPE_WHITE_DWARF) {
				// white dwarfs will have started as stars < 8 solar
				// masses or so, so pick max according to that
				// We give it a larger min because it used to be a much larger star
				min = 1000 * primary->radius * AU_SOL_RADIUS;
				max = 100 * rand.NFixed(2);		// rand-splitting again
				max *= fixed::SqrtOf(fixed(1,2) + fixed(8,1)*rand.Fixed());
			} else {
				max = 100 * rand.NFixed(2)*fixed::SqrtOf(primary->mass);
			}
			// having limited min by bin-separation/fake roche, and
			// max by some relation to star mass, we can now compute
			// disc density
			density = rand.Fixed() * GetDiscDensity(primary, min, max,ACCRETIONDISC_MASSRATIO_STAR);
			
			if ((superType == SystemBody::SUPERTYPE_STAR) && (primary->parent)) {
				// limit planets out to 10% distance to star's binary companion
				max = std::min(max, primary->orbMin * ACCRETIONDISC_RIM_LIMIT_BINARY);
			}

			/* in trinary and quaternary systems don't bump into other pair... */
			if (numStars >= 3) {
				max = std::min(max, rootBody->children[0]->orbMin * ACCRETIONDISC_RIM_LIMIT_TRINARY);
			}
		} else {
			fixed primary_rad = primary->radius * AU_EARTH_RADIUS;
			min = 4 * primary_rad;
			/* use hill radius to find max size of moon system. for stars botch it.
			   And use planets orbit around its primary as a scaler to a moon's orbit*/
			max = std::min(max, ACCRETIONDISC_MYSTERY_RATIO1 *
				primary->CalcHillRadius()*primary->orbMin*ACCRETIONDISC_MYSTERY_RATIO2);
			density = rand.Fixed() * GetDiscDensity(primary, min, max,ACCRETIONDISC_MASSRATIO_PLANET);
		}	
	}

	void SystemGenerator::FormPlanetInOrbit(BodyList& bodies, SystemBody* primary, AccretionDisc disc
		, fixed pos, fixed periapsis, fixed ecc, fixed semiMajorAxis, fixed apoapsis, int idx)
	{
		// get mass
		fixed mass = disc.AccretableMass(pos, MIN_PLANET_SEPARATION*apoapsis) * systemRand().Fixed();	
		if (mass < 0) {// hack around overflow
			fprintf(stderr, "WARNING: planetary mass has overflowed! (child of %s)\n", primary->name.c_str());
			mass = fixed(Sint64(0x7fFFffFFffFFffFFull));
		}
		assert(mass >= 0);

		// get name
		const bool isMoon = !(primary->GetSuperType() <= SystemBody::SUPERTYPE_STAR);
		char buf[8];
		if (isMoon) { snprintf(buf, sizeof(buf), " %d",   1+idx); } // moon name
		else        { snprintf(buf, sizeof(buf), " %c", 'a'+idx); } // planet name
		std::string name = primary->name+buf;

		// make the system body
		SystemBody *planet = NewBody(bodies, primary, name, SystemBody::TYPE_PLANET_TERRESTRIAL);
		planet->eccentricity   = ecc;
		planet->axialTilt      = fixed(100,157)*systemRand().NFixed(2);
		planet->semiMajorAxis  = semiMajorAxis;
		planet->seed           = systemRand().Int32();
		planet->tmp            = 0;
		planet->mass           = mass;
		planet->rotationPeriod = fixed(systemRand().Int32(1,200), 24);

		planet->orbit.eccentricity  = ecc.ToDouble();
		planet->orbit.semiMajorAxis = semiMajorAxis.ToDouble() * AU;
		planet->orbit.period        = calc_orbital_period(planet->orbit.semiMajorAxis, primary->GetMass());

		double r1 = systemRand().Double(2*M_PI);		// function parameter evaluation order is implementation-dependent
		double r2 = systemRand().NDouble(5);			// can't put two rands in the same expression
		planet->orbit.rotMatrix = matrix4x4d::RotateYMatrix(r1) *
			matrix4x4d::RotateXMatrix(-0.5*M_PI + r2*M_PI/2.0);

		planet->orbMin = periapsis;
		planet->orbMax = apoapsis;
	
		// add it to the primary's children
		primary->children.push_back(planet);
	}

	void SystemGenerator::AddPlanetsAround(BodyList& bodies, SystemBody* primary)
	{
		MTRand &rand = systemRand();
		AccretionDisc disc = AccretionDisc(m_rootBody, primary, NumStars(), rand);

		const fixed initialJump = rand.NFixed(5);
		fixed pos = (fixed(1,1) - initialJump)*disc.min + (initialJump*disc.max);

		// form the planets...
		int  idx=0;
		while (pos < disc.max) {
			// periapsis, apoapsis = closest, farthest distance in orbit
			fixed periapsis     = pos + pos*fixed(1,2)*rand.NFixed(2);/* + jump */;
			fixed ecc           = rand.NFixed(3);
			fixed semiMajorAxis = periapsis / (fixed(1,1) - ecc);
			fixed apoapsis      = 2*semiMajorAxis - periapsis;
			if (apoapsis > disc.max) break;

			FormPlanetInOrbit(bodies, primary, disc, pos, periapsis, ecc, semiMajorAxis, apoapsis, idx);
			pos = apoapsis * MIN_PLANET_SEPARATION;
			idx++;
		}

		// ...then pick their types	
		const bool isPlanet = (primary->GetSuperType() <= SystemBody::SUPERTYPE_STAR);

		for (std::vector<SystemBody*>::iterator i = primary->children.begin(); i != primary->children.end(); ++i) {
			if ((*i)->GetSuperType() == SystemBody::SUPERTYPE_STAR) continue;    // ignore any stars already orbiting our primary
		
			(*i)->PickPlanetType(rand);
			if (isPlanet) AddPlanetsAround(bodies, (*i)); // add moons
		}
	}

//-----------------------------------------------------------------------------
// State

	MTRand& SystemGenerator::systemRand() 
	{
		if (!m_systemRand) {
			unsigned long _init[6] = { m_path.systemIndex, Uint32(m_path.sectorX), Uint32(m_path.sectorY), Uint32(m_path.sectorZ), UNIVERSE_SEED, SectorSystem().seed };
			m_systemRand = new MTRand(_init, 6);
		}
		return *m_systemRand;
	}

//-----------------------------------------------------------------------------
// Construction/Destruction

	SystemGenerator::SystemGenerator(SystemPath& path): m_path(path), m_systemRand(0), m_sector(m_path.sectorX, m_path.sectorY, m_path.sectorZ)
		, m_centGrav1(0), m_centGrav2(0), m_rootBody(0)
	{
		assert(m_path.systemIndex >= 0 && m_path.systemIndex < m_sector.m_systems.size());
	}

	SystemGenerator::~SystemGenerator() 
	{
		if (m_systemRand) delete m_systemRand;
	}

} // namespace SystemGenerator