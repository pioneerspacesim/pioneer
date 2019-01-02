-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent Faction')
	:description_short('Independent Faction')
	:description('Very little is currently known about The Independent Faction')
	:homeworld(-49,-30,-30,0,8)
	:foundingDate(3098)
	:expansionRate(1.05195)
	:military_name('Independent Battle Flight')
	:police_name('Independent Prefecture')
	:colour(0.0392157,0.87451,0.72549)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		30)
f:govtype_weight('LIBDEM',		30)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('SLAVES',		27)
f:illegal_goods_probability('HAND_WEAPONS',		44)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		37)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Independent Faction')


