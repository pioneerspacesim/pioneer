-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Dagger Systems')
	:description_short('Dagger Systems')
	:description('Very little is currently known about The Dagger Systems')
	:homeworld(-47,24,24,2,3)
	:foundingDate(3141)
	:expansionRate(1.5471)
	:military_name('Dagger Defense Wing')
	:police_name('Dagger Interior Ministry')
	:colour(0.886275,0.168627,0.0196078)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		33)
f:govtype_weight('DISORDER',		33)
f:govtype_weight('SOCDEM',		10)

f:illegal_goods_probability('hand_weapons',		60)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		85)

f:add_to_factions('Dagger Systems')
