-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Inner Territories')
	:description_short('Inner Territories')
	:description('Very little is currently known about The Inner Territories')
	:homeworld(-24,38,38,0,11)
	:foundingDate(3159)
	:expansionRate(1.75779)
	:military_name('Inner Space Patrol')
	:police_name('Territories Inquisition')
	:colour(0.862745,0.364706,1)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		64)
f:govtype_weight('PLUTOCRATIC',		64)

f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('hand_weapons',		46)
f:illegal_goods_probability('battle_weapons',		67)
f:illegal_goods_probability('nerve_gas',		41)
f:illegal_goods_probability('narcotics',		77)

f:add_to_factions('Inner Territories')
