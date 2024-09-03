// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Polit.h"
#include "Factions.h"
#include "Lang.h"
#include "StringF.h"
#include "galaxy/Economy.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"

namespace Polit {

	const char *s_econDesc[ECON_MAX] = {
		Lang::NO_ESTABLISHED_ORDER,
		Lang::HARD_CAPITALIST,
		Lang::CAPITALIST,
		Lang::MIXED_ECONOMY,
		Lang::PLANNED_ECONOMY
	};

	struct politDesc_t {
		const char *description;
		int rarity;
		PolitEcon econ;
		fixed baseLawlessness;
	};
	static politDesc_t s_govDesc[GOV_MAX] = {
		{ "<invalid turd>", 0, ECON_NONE, fixed(1, 1) },
		{ Lang::NO_CENTRAL_GOVERNANCE, 0, ECON_NONE, fixed(1, 1) },
		{ Lang::EARTH_FEDERATION_COLONIAL_RULE, 2, ECON_CAPITALIST, fixed(3, 10) },
		{ Lang::EARTH_FEDERATION_DEMOCRACY, 3, ECON_CAPITALIST, fixed(15, 100) },
		{ Lang::IMPERIAL_RULE, 3, ECON_PLANNED, fixed(15, 100) },
		{ Lang::LIBERAL_DEMOCRACY, 2, ECON_CAPITALIST, fixed(25, 100) },
		{ Lang::SOCIAL_DEMOCRACY, 2, ECON_MIXED, fixed(20, 100) },
		{ Lang::LIBERAL_DEMOCRACY, 2, ECON_CAPITALIST, fixed(25, 100) },
		{ Lang::CORPORATE_SYSTEM, 2, ECON_CAPITALIST, fixed(40, 100) },
		{ Lang::SOCIAL_DEMOCRACY, 2, ECON_MIXED, fixed(25, 100) },
		{ Lang::MILITARY_DICTATORSHIP, 5, ECON_CAPITALIST, fixed(40, 100) },
		{ Lang::MILITARY_DICTATORSHIP, 6, ECON_CAPITALIST, fixed(25, 100) },
		{ Lang::MILITARY_DICTATORSHIP, 6, ECON_MIXED, fixed(25, 100) },
		{ Lang::MILITARY_DICTATORSHIP, 5, ECON_MIXED, fixed(40, 100) },
		{ Lang::COMMUNIST, 10, ECON_PLANNED, fixed(25, 100) },
		{ Lang::PLUTOCRATIC_DICTATORSHIP, 4, ECON_VERY_CAPITALIST, fixed(45, 100) },
		{ Lang::VIOLENT_ANARCHY, 2, ECON_NONE, fixed(90, 100) },
	};

	fixed GetBaseLawlessness(GovType gov)
	{
		return s_govDesc[gov].baseLawlessness;
	}

} // namespace Polit

const char *SysPolit::GetGovernmentDesc() const
{
	return Polit::s_govDesc[govType].description;
}

const char *SysPolit::GetEconomicDesc() const
{
	return Polit::s_econDesc[Polit::s_govDesc[govType].econ];
}
