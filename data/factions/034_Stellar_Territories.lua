-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Stellar Territories')
	:description_short('Stellar Territories')
	:description('Very little is currently known about The Stellar Territories')
	:homeworld(11,19,19,0,9)
	:foundingDate(3052)
	:expansionRate(0.530665)
	:military_name('Territories Defense Wing')
	:police_name('Territories Inquisition')
	:colour(0.792157,0.168627,0.941177)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		36)
f:govtype_weight('COMMUNIST',		13)
f:govtype_weight('SOCDEM',		4)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)

f:add_to_factions('Stellar Territories')


