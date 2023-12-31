-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Stellar Network')
	:description_short('Stellar Network')
	:description('Very little is currently known about The Stellar Network')
	:homeworld(-52,7,7,1,6)
	:foundingDate(3087)
	:expansionRate(0.924895)
	:military_name('Stellar Regiments')
	:police_name('Stellar Prefecture')
	:colour(0.980392,0.658824,0.360784)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		77)
f:govtype_weight('LIBDEM',		77)
f:govtype_weight('MILDICT1',		59)
f:govtype_weight('SOCDEM',		59)

f:illegal_goods_probability('liquor',		49)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('battle_weapons',		64)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Stellar Network')
