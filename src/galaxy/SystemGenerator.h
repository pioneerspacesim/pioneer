// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMGENERATOR_H
#define _SYSTEMGENERATOR_H

#include "libs.h"
#include "mtrand.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"

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

	const std::string Name()         const { return SectorSystem().name; }
	const bool        IsCustom()	 const { return SectorSystem().customSys; }
	const int         NumStars()     const { return SectorSystem().numStars;  }
	const bool        Unexplored();
	const fixed       Metallicity()  const;
	
	SystemBody*       AddStarsTo  (BodyList& bodies);
	void              AddPlanetsTo(BodyList& bodies);

	//---------------------- state accessors (eventually make private or eliminate)
	      MTRand&        rand1();
	const CustomSystem * custom()    { return SectorSystem().customSys; }

private:
	//---------------------- private state (eliminate where possible)
	SystemPath  m_path;
	Sector      m_sector;
	SystemBody* m_centGrav1;
	SystemBody* m_centGrav2;
	SystemBody* m_rootBody;

	MTRand*     m_rand1;

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

	const Sector::System SectorSystem() const { return m_sector.m_systems[m_path.systemIndex]; };
	
	const double calc_orbital_period(double semiMajorAxis, double centralMass) const { 
		return 2.0*M_PI*sqrt((semiMajorAxis*semiMajorAxis*semiMajorAxis)/(G*centralMass)); 
	}

};

#endif