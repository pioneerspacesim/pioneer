-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Liberation Horde')
	:description_short('Liberation Horde')
	:description('Very little is currently known about The Liberation Horde')
	:homeworld(-9,7,7,1,4)
	:foundingDate(3170)
	:expansionRate(1.87452)
	:military_name('Horde Defense Wing')
	:police_name('Horde Security')
	:colour(0.407843,0.647059,0.513726)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		14)
f:govtype_weight('LIBDEM',		14)
f:govtype_weight('MILDICT1',		2)
f:govtype_weight('SOCDEM',		2)
f:govtype_weight('DISORDER',		0)
f:govtype_weight('COMMUNIST',		0)

f:illegal_goods_probability('ANIMAL_MEAT',		99)
f:illegal_goods_probability('ROBOTS',		80)
f:illegal_goods_probability('BATTLE_WEAPONS',		37)
f:illegal_goods_probability('NERVE_GAS',		80)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Liberation Horde')


