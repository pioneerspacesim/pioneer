-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Free Republic')
	:description_short('Free Republic')
	:description('Very little is currently known about The Free Republic')
	:homeworld(40,26,26,1,2)
	:foundingDate(3178)
	:expansionRate(1.96808)
	:military_name('Republic Defense Wing')
	:police_name('Free Inquisition')
	:colour(0.960784,0.180392,0.113725)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		28)
f:govtype_weight('SOCDEM',		28)
f:govtype_weight('PLUTOCRATIC',		8)
f:govtype_weight('COMMUNIST',		8)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('nerve_gas',		84)
f:illegal_goods_probability('narcotics',		73)

f:add_to_factions('Free Republic')
