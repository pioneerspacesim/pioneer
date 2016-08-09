-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Outer Worlds')
	:description_short('Outer Worlds')
	:description('Very little is currently known about The Outer Worlds')
	:homeworld(-40,46,46,1,6)
	:foundingDate(3177)
	:expansionRate(1.95607)
	:military_name('Outer Defense Force')
	:police_name('Worlds Police')
	:colour(1,1,0.203922)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		36)
f:govtype_weight('SOCDEM',		36)
f:govtype_weight('PLUTOCRATIC',		13)
f:govtype_weight('COMMUNIST',		13)
f:govtype_weight('MILDICT1',		4)
f:govtype_weight('MILDICT2',		4)

f:illegal_goods_probability('ANIMAL_MEAT',		61)
f:illegal_goods_probability('LIVE_ANIMALS',		51)
f:illegal_goods_probability('ROBOTS',		51)
f:illegal_goods_probability('SLAVES',		98)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Outer Worlds')


