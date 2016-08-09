-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood Worlds')
	:description_short('Blood Worlds')
	:description('Very little is currently known about The Blood Worlds')
	:homeworld(36,43,43,0,4)
	:foundingDate(3050)
	:expansionRate(0.509812)
	:military_name('Worlds Navy')
	:police_name('Worlds Interior Ministry')
	:colour(0.243137,1,1)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		32)
f:govtype_weight('COMMUNIST',		10)
f:govtype_weight('SOCDEM',		3)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		37)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		76)
f:illegal_goods_probability('HAND_WEAPONS',		75)
f:illegal_goods_probability('NERVE_GAS',		50)

f:add_to_factions('Blood Worlds')


