// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "galaxy/Economy.h"
#include "Lang.h"

namespace GalacticEconomy {

const CommodityInfo COMMODITY_DATA[COMMODITY_COUNT] = {
	{
	  Lang::COMMODITY_NONE,
	  {},
	  0
	},{
	  Lang::HYDROGEN,
	  {},
	  ECON_MINING
	},{
	  Lang::LIQUID_OXYGEN,
	  { Commodity::WATER, Commodity::INDUSTRIAL_MACHINERY },
	  ECON_MINING
	},{
	  Lang::METAL_ORE,
	  { Commodity::MINING_MACHINERY },
	  ECON_MINING
	},{
	  Lang::CARBON_ORE,
	  { Commodity::MINING_MACHINERY },
	  ECON_MINING
	},{
	  Lang::METAL_ALLOYS,
	  { Commodity::METAL_ORE, Commodity::INDUSTRIAL_MACHINERY },
	  ECON_INDUSTRY
	},{
	  Lang::PLASTICS,
	  { Commodity::CARBON_ORE, Commodity::INDUSTRIAL_MACHINERY },
	  ECON_INDUSTRY
	},{
	  Lang::FRUIT_AND_VEG,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  ECON_AGRICULTURE
	},{
	  Lang::ANIMAL_MEAT,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  ECON_AGRICULTURE
	},{
	  Lang::LIVE_ANIMALS,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  ECON_AGRICULTURE
	},{
	  Lang::LIQUOR,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  ECON_AGRICULTURE
	},{
	  Lang::GRAIN,
	  { Commodity::FARM_MACHINERY, Commodity::FERTILIZER },
	  ECON_AGRICULTURE
	},{
	  Lang::TEXTILES,
	  { Commodity::PLASTICS },
	  ECON_INDUSTRY
	},{
	  Lang::FERTILIZER,
	  { Commodity::CARBON_ORE },
	  ECON_INDUSTRY
	},{
	  Lang::WATER,
	  { Commodity::MINING_MACHINERY },
	  ECON_MINING
	},{
	  Lang::MEDICINES,
	  { Commodity::COMPUTERS, Commodity::CARBON_ORE },
	  ECON_INDUSTRY
	},{
	  Lang::CONSUMER_GOODS,
	  { Commodity::PLASTICS, Commodity::TEXTILES },
	  ECON_INDUSTRY
	},{
	  Lang::COMPUTERS,
	  { Commodity::PRECIOUS_METALS, Commodity::INDUSTRIAL_MACHINERY },
	  ECON_INDUSTRY
	},{
	  Lang::ROBOTS,
	  { Commodity::PLASTICS, Commodity::COMPUTERS },
	  ECON_INDUSTRY
	},{
	  Lang::PRECIOUS_METALS,
	  { Commodity::MINING_MACHINERY },
	  ECON_MINING
	},{
	  Lang::INDUSTRIAL_MACHINERY,
	  { Commodity::METAL_ALLOYS, Commodity::ROBOTS },
	  ECON_INDUSTRY
	},{
	  Lang::FARM_MACHINERY,
	  { Commodity::METAL_ALLOYS, Commodity::ROBOTS },
	  ECON_INDUSTRY
	},{
	  Lang::MINING_MACHINERY,
	  { Commodity::METAL_ALLOYS, Commodity::ROBOTS },
	  ECON_INDUSTRY
	},{
	  Lang::AIR_PROCESSORS,
	  { Commodity::PLASTICS, Commodity::INDUSTRIAL_MACHINERY },
	  ECON_INDUSTRY
	},{
	  Lang::SLAVES,
	  {},
	  ECON_AGRICULTURE
	},{
	  Lang::HAND_WEAPONS,
	  { Commodity::COMPUTERS },
	  ECON_INDUSTRY
	},{
	  Lang::BATTLE_WEAPONS,
	  { Commodity::INDUSTRIAL_MACHINERY, Commodity::METAL_ALLOYS },
	  ECON_INDUSTRY
	},{
	  Lang::NERVE_GAS,
	  {},
	  ECON_INDUSTRY
	},{
	  Lang::NARCOTICS,
	  {},
	  ECON_INDUSTRY
	},{
	  Lang::MILITARY_FUEL,
	  { Commodity::HYDROGEN },
	  ECON_INDUSTRY
	},{
	  Lang::RUBBISH,
	  {},
	  ECON_INDUSTRY
	},{
	  Lang::RADIOACTIVES,
	  {},
	  ECON_INDUSTRY
	}
};

} // namespace GalacticEconomy
