-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Stellar Union')
	:description_short('Stellar Union')
	:description('Very little is currently known about The Stellar Union')
	:homeworld(44,12,12,1,7)
	:foundingDate(3124)
	:expansionRate(1.35647)
	:military_name('Union Battle Flight')
	:police_name('Stellar Prefecture')
	:colour(0.431373,0.741176,0.87451)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		11)
f:govtype_weight('DISORDER',		11)
f:govtype_weight('SOCDEM',		1)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIQUOR',		89)
f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('SLAVES',		46)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		100)

f:add_to_factions('Stellar Union')


