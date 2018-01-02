-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim Systems')
	:description_short('Rim Systems')
	:description('Very little is currently known about The Rim Systems')
	:homeworld(31,-58,-58,0,3)
	:foundingDate(3067)
	:expansionRate(0.697619)
	:military_name('Systems Space Arm')
	:police_name('Systems Constabulary')
	:colour(0.196078,0.254902,0.984314)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		70)
f:govtype_weight('DISORDER',		70)
f:govtype_weight('SOCDEM',		49)

f:illegal_goods_probability('ANIMAL_MEAT',		66)
f:illegal_goods_probability('LIVE_ANIMALS',		68)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		27)
f:illegal_goods_probability('NERVE_GAS',		49)
f:illegal_goods_probability('NARCOTICS',		86)

f:add_to_factions('Rim Systems')


