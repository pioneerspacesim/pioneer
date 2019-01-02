-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Shattered Network')
	:description_short('Shattered Network')
	:description('Very little is currently known about The Shattered Network')
	:homeworld(27,-61,-60,1,12)
	:foundingDate(3163)
	:expansionRate(1.80198)
	:military_name('Network Defense Wing')
	:police_name('Network Interior Ministry')
	:colour(0.309804,0.694118,0.686275)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		52)
f:govtype_weight('COMMUNIST',		27)
f:govtype_weight('SOCDEM',		14)

f:illegal_goods_probability('ANIMAL_MEAT',		27)
f:illegal_goods_probability('SLAVES',		51)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		54)

f:add_to_factions('Shattered Network')


