-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Commonwealth')
	:description_short('True Commonwealth')
	:description('Very little is currently known about The True Commonwealth')
	:homeworld(-49,52,52,0,8)
	:foundingDate(3099)
	:expansionRate(1.06292)
	:military_name('Commonwealth Battle Flight')
	:police_name('Commonwealth Constabulary')
	:colour(0.713726,0.541176,0.129412)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		74)
f:govtype_weight('LIBDEM',		74)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		55)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		33)
f:illegal_goods_probability('narcotics',		57)

f:add_to_factions('True Commonwealth')
