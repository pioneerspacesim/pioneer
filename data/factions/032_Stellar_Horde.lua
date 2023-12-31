-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Stellar Horde')
	:description_short('Stellar Horde')
	:description('Very little is currently known about The Stellar Horde')
	:homeworld(50,28,28,0,16)
	:foundingDate(3097)
	:expansionRate(1.04106)
	:military_name('Horde Battle Flight')
	:police_name('Stellar Interior Ministry')
	:colour(0.227451,0.380392,0.666667)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		47)
f:govtype_weight('LIBDEM',		47)
f:govtype_weight('MILDICT1',		22)
f:govtype_weight('SOCDEM',		22)

f:illegal_goods_probability('live_animals',		73)
f:illegal_goods_probability('slaves',		63)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		61)
f:illegal_goods_probability('nerve_gas',		41)

f:add_to_factions('Stellar Horde')
