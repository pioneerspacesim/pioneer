-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Dagger Council')
	:description_short('Dagger Council')
	:description('Very little is currently known about The Dagger Council')
	:homeworld(-25,-53,-53,0,1)
	:foundingDate(3066)
	:expansionRate(0.684718)
	:military_name('Council Navy')
	:police_name('Council Inquisition')
	:colour(0.662745,0.831373,0.678431)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		54)
f:govtype_weight('PLUTOCRATIC',		29)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('ROBOTS',		44)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		59)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NARCOTICS',		55)

f:add_to_factions('Dagger Council')


