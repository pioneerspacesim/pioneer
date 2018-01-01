-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		55)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		33)
f:illegal_goods_probability('NARCOTICS',		57)

f:add_to_factions('True Commonwealth')


