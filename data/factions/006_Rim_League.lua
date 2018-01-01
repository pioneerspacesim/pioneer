-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('LIVE_ANIMALS',		93)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		39)
f:illegal_goods_probability('NERVE_GAS',		100)

f:add_to_factions('Rim League')


