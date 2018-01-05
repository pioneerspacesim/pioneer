-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Inner Faction')
	:description_short('Inner Faction')
	:description('Very little is currently known about The Inner Faction')
	:homeworld(-55,-9,-9,0,2)
	:foundingDate(3096)
	:expansionRate(1.0357)
	:military_name('Faction Battle Flight')
	:police_name('Faction Interior Ministry')
	:colour(1,0.866667,0.807843)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		8)
f:govtype_weight('SOCDEM',		8)
f:govtype_weight('PLUTOCRATIC',		0)
f:govtype_weight('COMMUNIST',		0)
f:govtype_weight('MILDICT1',		0)
f:govtype_weight('MILDICT2',		0)

f:illegal_goods_probability('ANIMAL_MEAT',		97)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('ROBOTS',		81)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		54)

f:add_to_factions('Inner Faction')


