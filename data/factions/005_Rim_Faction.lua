-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim Faction')
	:description_short('Rim Faction')
	:description('Very little is currently known about The Rim Faction')
	:homeworld(-38,-24,-24,0,15)
	:foundingDate(3092)
	:expansionRate(0.986974)
	:military_name('Faction Regiments')
	:police_name('Faction Justiciars')
	:colour(1,0.996078,1)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		71)
f:govtype_weight('PLUTOCRATIC',		71)
f:govtype_weight('CORPORATE',		50)
f:govtype_weight('LIBDEM',		35)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('liquor',		69)
f:illegal_goods_probability('robots',		57)
f:illegal_goods_probability('slaves',		82)
f:illegal_goods_probability('hand_weapons',		28)
f:illegal_goods_probability('battle_weapons',		71)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		53)

f:add_to_factions('Rim Faction')
