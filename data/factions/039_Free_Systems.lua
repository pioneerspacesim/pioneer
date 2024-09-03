-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('liquor',		26)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('battle_weapons',		41)
f:illegal_goods_probability('nerve_gas',		68)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Free Systems')
