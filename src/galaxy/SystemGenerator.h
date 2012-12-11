// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMGENERATOR_H
#define _SYSTEMGENERATOR_H

#include "libs.h"
#include "mtrand.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"
#include "Factions.h"


namespace SystemGeneration
{

	static const fixed ACCRETIONDISC_MASSRATIO_STAR    = fixed(2,100);   // Mass of the protoplanetary disc in relation to the star
	static const fixed ACCRETIONDISC_MASSRATIO_PLANET  = fixed(1,500);   // bump this up to 20/500 or 25/500ish to see earth mass moons around gas giants
	static const fixed ACCRETIONDISC_RIM_LIMIT_BINARY  = fixed(1,10);    // how far towards the companion star the protoplanetary disc can reach in a binary system
	static const fixed ACCRETIONDISC_RIM_LIMIT_TRINARY = fixed(5,100);   // similar for trinary and quarternary
	static const fixed ACCRETIONDISC_MYSTERY_RATIO1    = fixed(1,20);    // ...it's *something* to do with the accretion disc outer limit for moon systems
	static const fixed ACCRETIONDISC_MYSTERY_RATIO2    = fixed(1,10);    // ditto
	
	static const fixed ACCRETIONDISC_SAFE_BINARY_DIST  = fixed(5,1);     // if binary stars have separation s, planets can have stable
	                                                                     // orbits at (0.5 * s * ACCRETIONDISC_SAFE_BINARY_DIST)

	static const fixed MIN_PLANET_SEPARATION  = fixed(135,100);          // I _think_ >1 means planets are more massive and have greater separation with distance


	class AccretionDisc;

	/*
	 * Functionality:
	 *   - Generate Star System detail for a <StarSystem> based on its <SystemPath>
	 */

	class SystemGenerator {
	public:
		SystemGenerator(SystemPath& path);
		~SystemGenerator();
	 
		//---------------------- builders
		typedef std::vector<SystemBody*> BodyList;

		const std::string   Name()         const { return SectorSystem().name; }
		const CustomSystem* Custom()       const { return SectorSystem().customSys; }
		      Faction*      Faction()      const { return Faction::GetNearestFaction(m_sector, m_path.systemIndex); }
	
		const int           NumStars()     const;
		const bool          Unexplored();
		const fixed         Industry()           { return populationRand().Fixed(); }
		      fixed         HumanProx()    const;    // eventually make private and remove from StarSystem as only used in generation
		const fixed         Metallicity()  const;

		SystemBody*         AddStarsTo     (BodyList& bodies);
		void                AddPlanetsTo   (BodyList& bodies);
		const fixed         AddPopulationTo(StarSystem* system);

		//---------------------- state accessors (eventually make private or eliminate)
		MTRand& systemRand();

	private:
		//---------------------- private state (eliminate where possible)
		SystemPath  m_path;
		Sector      m_sector;
		SystemBody* m_centGrav1;
		SystemBody* m_centGrav2;
		SystemBody* m_rootBody;

		MTRand*     m_systemRand;
		MTRand*     m_populationRand;

		//---------------------- private builders
	
		// stars
		SystemBody* AddStarOfType           (BodyList& bodies, std::string name, SystemBody::BodyType type);
		SystemBody* AddStarOfTypeLighterThan(BodyList& bodies, std::string name, SystemBody::BodyType type, fixed maxMass);
		SystemBody* AddGravPoint            (BodyList& bodies, std::string name, SystemBody* a, SystemBody* b, bool limitOrbit);
		SystemBody* AddGravPoint            (BodyList& bodies, std::string name, SystemBody* a, SystemBody* b, bool limitOrbit, fixed minDist);
		void MakeStar(SystemBody *sbody);
		void MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist);

		// planets
		void AddPlanetsAround(BodyList& bodies, SystemBody* primary);
		void FormPlanetInOrbit(BodyList& bodies, SystemBody* primary, AccretionDisc disc, fixed pos, fixed periapsis, fixed ecc, fixed semiMajorAxis, fixed apoapsis, int idx);

		// bodies
		SystemBody *NewBody(BodyList& bodies, SystemBody* parent, std::string name, SystemBody::BodyType type) {
			SystemBody *body = new SystemBody;
			body->path = m_path;
			body->parent = parent;
			body->name   = name;
			body->type   = type;
			body->orbMin = 0;
			body->orbMax = 0;
			body->path.bodyIndex = bodies.size();
			bodies.push_back(body);
			return body;
		}

		//---------------------- private helpers
		MTRand& populationRand();

		const Sector::System SectorSystem() const { return m_sector.m_systems[m_path.systemIndex]; };
	
		const double calc_orbital_period(double semiMajorAxis, double centralMass) const { 
			return 2.0*M_PI*sqrt((semiMajorAxis*semiMajorAxis*semiMajorAxis)/(G*centralMass)); 
		}
	};

} // namespace SystemGenerator
#endif