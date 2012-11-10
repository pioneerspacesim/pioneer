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
 *   - Generate Star System detail for a <StarSystem> based on that <StarSystem's> path
 */

class SystemGenerator {

public:
	SystemGenerator(SystemPath& path);
	~SystemGenerator();
	 
	// builders
	const std::string Name();
	const bool        Unexplored();
	const bool        IsCustom();	
	SystemBody*       AddStarsTo(std::vector<SystemBody*>& bodies);

	// state (eventually make private or eliminate)
	      MTRand&        rand1();
	const Sector&        sector() { return m_sector; }
	const CustomSystem * custom();
	      SystemBody*    centGrav1() { return m_centGrav1; };
	      SystemBody*    centGrav2() { return m_centGrav2; };
	const int            numStars()  { return m_numStars;  };

private:
	// state (eliminate where possible)
	SystemPath  m_path;
	Sector      m_sector;
	int         m_seed;
	int         m_numStars;
	SystemBody* m_centGrav1;
	SystemBody* m_centGrav2;

	MTRand*     m_rand1;

	// private builders
	void MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type);
	void MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass);
	void MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist);

	SystemBody *NewBody(std::vector<SystemBody*>& bodies) {
		SystemBody *body = new SystemBody;
		body->path = m_path;
		body->path.bodyIndex = bodies.size();
		bodies.push_back(body);
		return body;
	}
};

#endif