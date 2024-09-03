-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent Commonwealth')
	:description_short('Independent Commonwealth')
	:description('Very little is currently known about The Independent Commonwealth')
	:homeworld(-11,30,30,0,1)
	:foundingDate(3147)
	:expansionRate(1.61175)
	:military_name('Commonwealth Space Arm')
	:police_name('Independent Constabulary')
	:colour(0.733333,1,0.294118)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		77)
f:govtype_weight('LIBDEM',		77)
f:govtype_weight('MILDICT1',		59)
f:govtype_weight('SOCDEM',		59)
f:govtype_weight('DISORDER',		45)
f:govtype_weight('COMMUNIST',		45)

f:illegal_goods_probability('animal_meat',		73)
f:illegal_goods_probability('robots',		73)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Independent Commonwealth')
