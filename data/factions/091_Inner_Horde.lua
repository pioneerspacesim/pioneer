-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Inner Horde')
	:description_short('Inner Horde')
	:description('Very little is currently known about The Inner Horde')
	:homeworld(-14,-30,-30,0,9)
	:foundingDate(3155)
	:expansionRate(1.71364)
	:military_name('Inner War Fleet')
	:police_name('Horde Constabulary')
	:colour(0.172549,0.584314,0.890196)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		25)
f:govtype_weight('CORPORATE',		25)
f:govtype_weight('DISORDER',		6)
f:govtype_weight('LIBDEM',		6)
f:govtype_weight('SOCDEM',		1)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('SLAVES',		48)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		89)
f:illegal_goods_probability('NERVE_GAS',		36)
f:illegal_goods_probability('NARCOTICS',		54)

f:add_to_factions('Inner Horde')


