// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _RINGSTYLE_H
#define _RINGSTYLE_H

#include "fixed.h"
#include "Color.h"

struct RingStyle {
	// note: radius values are given as proportions of the planet radius
	// (e.g., 1.6)
	fixed minRadius;
	fixed maxRadius;
	Color baseColor;
};

#endif /* _RINGSTYLE_H */
