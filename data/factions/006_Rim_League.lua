-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim League')
	:description_short('Rim League')
	:description('Very little is currently known about The Rim League')
	:homeworld(-11,-8,-8,0,8)
	:foundingDate(3072)
	:expansionRate(0.753644)
	:military_name('League Battle Flight')
	:police_name('Rim Prefecture')
	:colour(0.388235,1,0.741176)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		74)

f:illegal_goods_probability('live_animals',		93)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		39)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Rim League')
