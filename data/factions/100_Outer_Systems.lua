-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('animal_meat',		37)
f:illegal_goods_probability('live_animals',		44)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		49)
f:illegal_goods_probability('nerve_gas',		76)

f:add_to_factions('Outer Systems')
