-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Shattered Empire')
	:description_short('Shattered Empire')
	:description('Very little is currently known about The Shattered Empire')
	:homeworld(-7,-23,-23,1,32)
	:foundingDate(3160)
	:expansionRate(1.76983)
	:military_name('Empire Regiments')
	:police_name('Shattered Justiciars')
	:colour(0.227451,0.792157,1)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		10)
f:govtype_weight('SOCDEM',		10)
f:govtype_weight('PLUTOCRATIC',		1)
f:govtype_weight('COMMUNIST',		1)

f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		54)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		93)

f:add_to_factions('Shattered Empire')


