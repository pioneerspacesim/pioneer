-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Outer Expanse')
	:description_short('Outer Expanse')
	:description('Very little is currently known about The Outer Expanse')
	:homeworld(-13,-13,-12,0,11)
	:foundingDate(3135)
	:expansionRate(1.48027)
	:military_name('Expanse War Fleet')
	:police_name('Outer Inquisition')
	:colour(1,1,1)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		23)
f:govtype_weight('COMMUNIST',		5)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		37)

f:add_to_factions('Outer Expanse')


