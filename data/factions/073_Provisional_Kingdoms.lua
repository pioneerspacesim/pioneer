-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Provisional Kingdoms')
	:description_short('Provisional Kingdoms')
	:description('Very little is currently known about The Provisional Kingdoms')
	:homeworld(-55,49,49,2,11)
	:foundingDate(3118)
	:expansionRate(1.28606)
	:military_name('Kingdoms War Fleet')
	:police_name('Provisional Interior Ministry')
	:colour(0.290196,1,0.407843)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		8)
f:govtype_weight('MILDICT2',		8)
f:govtype_weight('LIBDEM',		0)
f:govtype_weight('DISORDER',		0)
f:govtype_weight('CORPORATE',		0)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('SLAVES',		25)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		87)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Provisional Kingdoms')


