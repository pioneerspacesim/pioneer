-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Free Systems')
	:description_short('Free Systems')
	:description('Very little is currently known about The Free Systems')
	:homeworld(-18,-56,-56,0,9)
	:foundingDate(3050)
	:expansionRate(0.510286)
	:military_name('Systems Battle Flight')
	:police_name('Free Prefecture')
	:colour(0.411765,0.294118,0.345098)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		51)
f:govtype_weight('COMMUNIST',		51)

f:illegal_goods_probability('LIQUOR',		26)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		41)
f:illegal_goods_probability('NERVE_GAS',		68)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Free Systems')


