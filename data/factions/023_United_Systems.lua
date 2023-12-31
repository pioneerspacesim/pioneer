-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Systems')
	:description_short('United Systems')
	:description('Very little is currently known about The United Systems')
	:homeworld(-28,3,3,0,5)
	:foundingDate(3056)
	:expansionRate(0.57848)
	:military_name('United Regiments')
	:police_name('United Security')
	:colour(0.478431,0.964706,0.207843)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		54)
f:govtype_weight('DISORDER',		54)
f:govtype_weight('SOCDEM',		29)
f:govtype_weight('LIBDEM',		15)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('slaves',		56)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		73)
f:illegal_goods_probability('nerve_gas',		86)

f:add_to_factions('United Systems')
