// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _POLIT_H
#define _POLIT_H

#include "fixed.h"
#include "galaxy/Economy.h"

class Galaxy;
class StarSystem;
class SysPolit;
class Ship;

namespace Polit {
	enum PolitEcon { // <enum scope='Polit' name=PolitEcon prefix=ECON_ public>
		ECON_NONE,
		ECON_VERY_CAPITALIST,
		ECON_CAPITALIST,
		ECON_MIXED,
		ECON_PLANNED,
		ECON_MAX // <enum skip>
	};

	enum GovType { // <enum scope='Polit' name=PolitGovType prefix=GOV_ public>
		GOV_INVALID, // <enum skip>
		GOV_NONE,
		GOV_EARTHCOLONIAL,
		GOV_EARTHDEMOC,
		GOV_EMPIRERULE,
		GOV_CISLIBDEM,
		GOV_CISSOCDEM,
		GOV_LIBDEM,
		GOV_CORPORATE,
		GOV_SOCDEM,
		GOV_EARTHMILDICT,
		GOV_MILDICT1,
		GOV_MILDICT2,
		GOV_EMPIREMILDICT,
		GOV_COMMUNIST,
		GOV_PLUTOCRATIC,
		GOV_DISORDER,
		GOV_MAX, // <enum skip>
		GOV_RAND_MIN = GOV_NONE + 1, // <enum skip>
		GOV_RAND_MAX = GOV_MAX - 1, // <enum skip>
	};

	fixed GetBaseLawlessness(GovType gov);
} // namespace Polit

class SysPolit {
public:
	SysPolit() :
		govType(Polit::GOV_INVALID) {}

	const char *GetGovernmentDesc() const;
	const char *GetEconomicDesc() const;

	Polit::GovType govType;
	fixed lawlessness;
};

#endif /* _POLIT_H */
