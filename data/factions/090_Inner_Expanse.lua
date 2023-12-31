-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Inner Expanse')
	:description_short('Inner Expanse')
	:description('Very little is currently known about The Inner Expanse')
	:homeworld(11,29,29,2,23)
	:foundingDate(3136)
	:expansionRate(1.48807)
	:military_name('Inner Guards')
	:police_name('Expanse Inquisition')
	:colour(0.560784,0.188235,0.952941)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		36)
f:govtype_weight('DISORDER',		36)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Inner Expanse')
