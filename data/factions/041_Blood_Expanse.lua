-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood Expanse')
	:description_short('Blood Expanse')
	:description('Very little is currently known about The Blood Expanse')
	:homeworld(22,-17,-17,0,15)
	:foundingDate(3164)
	:expansionRate(1.8076)
	:military_name('Expanse Battle Flight')
	:police_name('Blood Constabulary')
	:colour(0.764706,0.470588,0.466667)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		38)
f:govtype_weight('MILDICT2',		38)
f:govtype_weight('LIBDEM',		14)
f:govtype_weight('DISORDER',		14)
f:govtype_weight('CORPORATE',		5)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		32)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		76)

f:add_to_factions('Blood Expanse')


