// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMGENERATOR_H
#define _SYSTEMGENERATOR_H

#include "libs.h"
#include "mtrand.h"
#include "galaxy/StarSystem.h"

/*
 * Functionality:
 *   - Generate Star System detail for a <StarSystem> based on that <StarSystem's> path
 */

class SystemGenerator {

public:
	SystemGenerator(SystemPath& path);
	~SystemGenerator();
	 
	const std::string Name();
	const bool        Unexplored();
	const bool        IsCustom();
	
	void MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type);
	void MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass);
	void MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist);

	// state (eventually make private)
	      MTRand&        rand1();
	const Sector&        sector() { return m_sector; }
	const CustomSystem * custom();

private:
	SystemPath m_path;
	Sector     m_sector;
	int        m_seed;	

	MTRand*    m_rand1;
};

#endif