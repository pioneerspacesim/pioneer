-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Druello Pirates')
	:description_short('Pirates banded together under the rule of Captain Oren Druello')
	:description('A loosely held together gang of pirates, ruled by Captain Oren Druello, a defector SolFed military officer.')
	:homeworld(-57,9,-21, 2, 0)
	:foundingDate(3148)
	:expansionRate(0.2)
	:military_name('Druello Marauders')
	:police_name('Enforcer')
	:colour(0.8,0.2,0.2)

f:govtype_weight('MILDICT2',80)
f:govtype_weight('DISORDER',20)

f:illegal_goods_probability('robots',100)		--Old man Druello is insanely paranoid about killer robots.

-- spread our wings and infest nearby starsystems with our faction
--f:claim(-56,10,-22)

f:add_to_factions('Druello Pirates')
