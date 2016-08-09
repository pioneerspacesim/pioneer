-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Worlds')
	:description_short('True Worlds')
	:description('Very little is currently known about The True Worlds')
	:homeworld(-25,-12,-12,0,12)
	:foundingDate(3104)
	:expansionRate(1.12542)
	:military_name('Worlds Legion')
	:police_name('Worlds Constabulary')
	:colour(0.0823529,0.133333,0.768628)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		27)
f:govtype_weight('CORPORATE',		27)
f:govtype_weight('DISORDER',		7)
f:govtype_weight('LIBDEM',		7)
f:govtype_weight('SOCDEM',		1)

f:illegal_goods_probability('ANIMAL_MEAT',		84)
f:illegal_goods_probability('SLAVES',		97)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		31)
f:illegal_goods_probability('NERVE_GAS',		100)

f:add_to_factions('True Worlds')


