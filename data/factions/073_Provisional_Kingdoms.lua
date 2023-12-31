-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Provisional Kingdoms')
	:description_short('Provisional Kingdoms')
	:description('Very little is currently known about The Provisional Kingdoms')
	:homeworld(-55,49,49,2,11)
	:foundingDate(3118)
	:expansionRate(1.28606)
	:military_name('Kingdoms War Fleet')
	:police_name('Provisional Interior Ministry')
	:colour(0.290196,1,0.407843)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		8)
f:govtype_weight('MILDICT2',		8)
f:govtype_weight('LIBDEM',		0)
f:govtype_weight('DISORDER',		0)
f:govtype_weight('CORPORATE',		0)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('slaves',		25)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		87)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Provisional Kingdoms')
