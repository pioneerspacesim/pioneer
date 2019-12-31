-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent Republic')
	:description_short('Independent Republic')
	:description('Very little is currently known about The Independent Republic')
	:homeworld(31,23,23,1,10)
	:foundingDate(3130)
	:expansionRate(1.41635)
	:military_name('Republic War Fleet')
	:police_name('Independent Police')
	:colour(1,1,0.129412)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		51)
f:govtype_weight('PLUTOCRATIC',		26)

f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('ROBOTS',		62)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		65)
f:illegal_goods_probability('NERVE_GAS',		100)

f:add_to_factions('Independent Republic')


