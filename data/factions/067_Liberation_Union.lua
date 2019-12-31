-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Liberation Union')
	:description_short('Liberation Union')
	:description('Very little is currently known about The Liberation Union')
	:homeworld(-24,-4,-3,0,7)
	:foundingDate(3118)
	:expansionRate(1.28201)
	:military_name('Union Regiments')
	:police_name('Liberation Justiciars')
	:colour(0.635294,1,1)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		69)
f:govtype_weight('COMMUNIST',		69)
f:govtype_weight('CORPORATE',		47)
f:govtype_weight('MILDICT2',		47)

f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		25)
f:illegal_goods_probability('BATTLE_WEAPONS',		72)
f:illegal_goods_probability('NERVE_GAS',		78)
f:illegal_goods_probability('NARCOTICS',		68)

f:add_to_factions('Liberation Union')


