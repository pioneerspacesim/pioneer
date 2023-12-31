-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Dagger Union')
	:description_short('Dagger Union')
	:description('Very little is currently known about The Dagger Union')
	:homeworld(-34,45,44,0,5)
	:foundingDate(3074)
	:expansionRate(0.78596)
	:military_name('Dagger Space Patrol')
	:police_name('Union Interior Ministry')
	:colour(1,0.505882,0.0431373)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		49)
f:govtype_weight('COMMUNIST',		24)
f:govtype_weight('SOCDEM',		11)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('liquor',		76)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		79)
f:illegal_goods_probability('battle_weapons',		33)
f:illegal_goods_probability('narcotics',		53)

f:add_to_factions('Dagger Union')
