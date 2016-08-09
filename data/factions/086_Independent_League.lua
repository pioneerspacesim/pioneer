-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent League')
	:description_short('Independent League')
	:description('Very little is currently known about The Independent League')
	:homeworld(32,10,10,1,7)
	:foundingDate(3071)
	:expansionRate(0.742945)
	:military_name('League Regiments')
	:police_name('Independent Justiciars')
	:colour(0.223529,0.937255,0.827451)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		69)
f:govtype_weight('PLUTOCRATIC',		69)

f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('LIQUOR',		50)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		53)
f:illegal_goods_probability('NARCOTICS',		81)

f:add_to_factions('Independent League')


