-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Outer Systems')
	:description_short('Outer Systems')
	:description('Very little is currently known about The Outer Systems')
	:homeworld(7,-33,-33,0,6)
	:foundingDate(3110)
	:expansionRate(1.18777)
	:military_name('Outer Militia')
	:police_name('Outer Justiciars')
	:colour(0.8,0.568627,0.941177)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		70)
f:govtype_weight('SOCDEM',		70)
f:govtype_weight('PLUTOCRATIC',		49)
f:govtype_weight('COMMUNIST',		49)
f:govtype_weight('MILDICT1',		34)
f:govtype_weight('MILDICT2',		34)

f:illegal_goods_probability('ANIMAL_MEAT',		37)
f:illegal_goods_probability('LIVE_ANIMALS',		44)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		49)
f:illegal_goods_probability('NERVE_GAS',		76)

f:add_to_factions('Outer Systems')


