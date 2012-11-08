// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SYSTEMGENERATOR_H
#define _SYSTEMGENERATOR_H

#include "libs.h"
#include "galaxy/StarSystem.h"

/*
 * Functionality:
 *   - Generate Star System detail for a <StarSystem> based on that <StarSystem's> path
 */

class SystemGenerator {

public:
	SystemGenerator();
	~SystemGenerator();

	void Generate(StarSystem& system);

private:

};

#endif