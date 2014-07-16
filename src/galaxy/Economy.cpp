// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "galaxy/Economy.h"
#include "Lang.h"

namespace GalacticEconomy {

const CommodityInfo COMMODITY_DATA[COMMODITY_COUNT] = {
	{
	  Lang::NONE, 0,
	  {},
	  0, 0
	},{
	  Lang::HYDROGEN, Lang::HYDROGEN_DESCRIPTION,
	  {},
	  100, ECON_MINING
	},{
	  Lang::LIQUID_OXYGEN, Lang::LIQUID_OXYGEN_DESCRIPTION,
	  { Commodity::WATER, Commodity::INDUSTRIAL_MACHINERY },
	  150, ECON_MINING
	},{
	  Lang::METAL_ORE, 0,
	  { Commodity::MINING_MACHINERY },
	  300, ECON_MINING
	},{
	  Lang::CARBON_ORE, Lang::CARBON_ORE_DESCRIPTION,
	  { Commodity::MINING_MACHINERY },
	  500, ECON_MINING
	},{
	  Lang::METAL_ALLOYS, 0,
	  { Commodity::METAL_ORE, Commodity::INDUSTRIAL_MACHINERY },
	  800, ECON_INDUSTRY
	},{
	  Lang::PLASTICS, 0,
	  { Commodity::CARBON_ORE, Commodity::INDUSTRIAL_MACHINERY },
	  1200, ECON_INDUSTRY
	},{
	  Lang::FRUIT_AND_VEG, 0,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  1200, ECON_AGRICULTURE
	},{
	  Lang::ANIMAL_MEAT, 0,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  1800, ECON_AGRICULTURE
	},{
	  Lang::LIVE_ANIMALS, 0,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  3200, ECON_AGRICULTURE
	},{
	  Lang::LIQUOR, 0,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  800, ECON_AGRICULTURE
	},{
	  Lang::GRAIN, 0,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  1000, ECON_AGRICULTURE
	},{
	  Lang::TEXTILES, 0,
	  { Commodity::PLASTICS },
	  850, ECON_INDUSTRY
	},{
	  Lang::FERTILIZER, 0,
	  { Commodity::CARBON_ORE },
	  400, ECON_INDUSTRY
	},{
	  Lang::WATER, 0,
	  { Commodity::MINING_MACHINERY },
	  120, ECON_MINING
	},{
	  Lang::MEDICINES,0,
	  { Commodity::COMPUTERS, Commodity::CARBON_ORE },
	  2200, ECON_INDUSTRY
	},{
	  Lang::CONSUMER_GOODS,0,
	  { Commodity::PLASTICS, Commodity::TEXTILES },
	  14000, ECON_INDUSTRY
	},{
	  Lang::COMPUTERS,0,
	  { Commodity::PRECIOUS_METALS, Commodity::INDUSTRIAL_MACHINERY },
	  8000, ECON_INDUSTRY
	},{
	  Lang::ROBOTS,0,
	  { Commodity::PLASTICS, Commodity::COMPUTERS },
	  6300, ECON_INDUSTRY
	},{
	  Lang::PRECIOUS_METALS, 0,
	  { Commodity::MINING_MACHINERY },
	  18000, ECON_MINING
	},{
	  Lang::INDUSTRIAL_MACHINERY,0,
	  { Commodity::METAL_ALLOYS, Commodity::ROBOTS },
	  1300, ECON_INDUSTRY
	},{
	  Lang::FARM_MACHINERY,0,
	  { Commodity::METAL_ALLOYS, Commodity::ROBOTS },
	  1100, ECON_INDUSTRY
	},{
	  Lang::MINING_MACHINERY,0,
	  { Commodity::METAL_ALLOYS, Commodity::ROBOTS },
	  1200, ECON_INDUSTRY
	},{
	  Lang::AIR_PROCESSORS,0,
	  { Commodity::PLASTICS, Commodity::INDUSTRIAL_MACHINERY },
	  2000, ECON_INDUSTRY
	},{
	  Lang::SLAVES,0,
	  {},
	  23200, ECON_AGRICULTURE
	},{
	  Lang::HAND_WEAPONS,0,
	  { Commodity::COMPUTERS },
	  12400, ECON_INDUSTRY
	},{
	  Lang::BATTLE_WEAPONS,0,
	  { Commodity::INDUSTRIAL_MACHINERY, Commodity::METAL_ALLOYS },
	  22000, ECON_INDUSTRY
	},{
	  Lang::NERVE_GAS,0,
	  {},
	  26500, ECON_INDUSTRY
	},{
	  Lang::NARCOTICS,0,
	  {},
	  15700, ECON_INDUSTRY
	},{
	  Lang::MILITARY_FUEL,0,
	  { Commodity::HYDROGEN },
	  6000, ECON_INDUSTRY
	},{
	  Lang::RUBBISH,0,
	  {},
	  -10, ECON_INDUSTRY
	},{
	  Lang::RADIOACTIVES,0,
	  {},
	  -35, ECON_INDUSTRY
	}
};

} // namespace GalacticEconomy
