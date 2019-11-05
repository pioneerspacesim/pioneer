// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StarSystemWriter.h"

#include "Lang.h"
#include "ExplorationState.h"
#include "StringF.h"
#include "utils.h"

void StarSystemWriter::MakeShortDescription()
{
	PROFILE_SCOPED()
	if (m_ssys->GetExplored() == ExplorationState::eUNEXPLORED)
		SetShortDesc(Lang::UNEXPLORED_SYSTEM_NO_DATA);

	else if (m_ssys->GetExplored() == ExplorationState::eEXPLORED_BY_PLAYER)
		SetShortDesc(stringf(Lang::RECENTLY_EXPLORED_SYSTEM, formatarg("date", format_date_only(m_ssys->GetExploredTime()))));

	/* Total population is in billions */
	else if (m_ssys->GetTotalPop() == 0) {
		SetShortDesc(Lang::SMALL_SCALE_PROSPECTING_NO_SETTLEMENTS);
	} else if (m_ssys->GetTotalPop() < fixed(1, 10)) {
		switch (m_ssys->GetEconType()) {
		case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::SMALL_INDUSTRIAL_OUTPOST); break;
		case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::SOME_ESTABLISHED_MINING); break;
		case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::YOUNG_FARMING_COLONY); break;
		}
	} else if (m_ssys->GetTotalPop() < fixed(1, 2)) {
		switch (m_ssys->GetEconType()) {
		case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::INDUSTRIAL_COLONY); break;
		case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::MINING_COLONY); break;
		case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::OUTDOOR_AGRICULTURAL_WORLD); break;
		}
	} else if (m_ssys->GetTotalPop() < fixed(5, 1)) {
		switch (m_ssys->GetEconType()) {
		case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::HEAVY_INDUSTRY); break;
		case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::EXTENSIVE_MINING); break;
		case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::THRIVING_OUTDOOR_WORLD); break;
		}
	} else {
		switch (m_ssys->GetEconType()) {
		case GalacticEconomy::ECON_INDUSTRY: SetShortDesc(Lang::INDUSTRIAL_HUB_SYSTEM); break;
		case GalacticEconomy::ECON_MINING: SetShortDesc(Lang::VAST_STRIP_MINE); break;
		case GalacticEconomy::ECON_AGRICULTURE: SetShortDesc(Lang::HIGH_POPULATION_OUTDOOR_WORLD); break;
		}
	}
}
