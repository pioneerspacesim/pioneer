-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Systems')
	:description_short('United Systems')
	:description('Very little is currently known about The United Systems')
	:homeworld(-28,3,3,0,5)
	:foundingDate(3056)
	:expansionRate(0.57848)
	:military_name('United Regiments')
	:police_name('United Security')
	:colour(0.478431,0.964706,0.207843)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		54)
f:govtype_weight('DISORDER',		54)
f:govtype_weight('SOCDEM',		29)
f:govtype_weight('LIBDEM',		15)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		56)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		73)
f:illegal_goods_probability('NERVE_GAS',		86)

f:add_to_factions('United Systems')


