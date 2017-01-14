-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Inner Council')
	:description_short('Inner Council')
	:description('Very little is currently known about The Inner Council')
	:homeworld(-14,58,58,1,12)
	:foundingDate(3085)
	:expansionRate(0.911935)
	:military_name('Inner Militia')
	:police_name('Council Inquisition')
	:colour(0.345098,0.913726,1)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		21)
f:govtype_weight('LIBDEM',		21)
f:govtype_weight('MILDICT1',		4)
f:govtype_weight('SOCDEM',		4)

f:illegal_goods_probability('ANIMAL_MEAT',		81)
f:illegal_goods_probability('LIQUOR',		68)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		97)

f:add_to_factions('Inner Council')


