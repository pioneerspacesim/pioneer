-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood Council')
	:description_short('Blood Council')
	:description('Very little is currently known about The Blood Council')
	:homeworld(-24,-15,-15,0,6)
	:foundingDate(3074)
	:expansionRate(0.780236)
	:military_name('Blood War Fleet')
	:police_name('Council Interior Ministry')
	:colour(1,0.662745,0.823529)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		66)
f:govtype_weight('LIBDEM',		66)
f:govtype_weight('MILDICT1',		44)
f:govtype_weight('SOCDEM',		44)
f:govtype_weight('DISORDER',		29)
f:govtype_weight('COMMUNIST',		29)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		86)
f:illegal_goods_probability('battle_weapons',		98)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Blood Council')
