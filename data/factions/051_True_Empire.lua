-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Empire')
	:description_short('True Empire')
	:description('Very little is currently known about The True Empire')
	:homeworld(-20,-32,-31,2,14)
	:foundingDate(3137)
	:expansionRate(1.5062)
	:military_name('Empire Defense Wing')
	:police_name('True Inquisition')
	:colour(0.454902,0.933333,0.596078)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		40)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('ROBOTS',		70)
f:illegal_goods_probability('SLAVES',		90)
f:illegal_goods_probability('HAND_WEAPONS',		52)
f:illegal_goods_probability('NERVE_GAS',		65)
f:illegal_goods_probability('NARCOTICS',		53)

f:add_to_factions('True Empire')


