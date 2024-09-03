-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood Kingdoms')
	:description_short('Blood Kingdoms')
	:description('Very little is currently known about The Blood Kingdoms')
	:homeworld(-53,52,52,0,9)
	:foundingDate(3089)
	:expansionRate(0.95249)
	:military_name('Blood Guards')
	:police_name('Blood Police')
	:colour(0.466667,0.164706,0.188235)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		13)
f:govtype_weight('COMMUNIST',		13)
f:govtype_weight('CORPORATE',		1)
f:govtype_weight('MILDICT2',		1)

f:illegal_goods_probability('animal_meat',		53)
f:illegal_goods_probability('live_animals',		50)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Blood Kingdoms')
