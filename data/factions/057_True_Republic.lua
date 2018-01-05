-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Republic')
	:description_short('True Republic')
	:description('Very little is currently known about The True Republic')
	:homeworld(39,-42,-42,0,8)
	:foundingDate(3053)
	:expansionRate(0.538303)
	:military_name('Republic Navy')
	:police_name('True Interior Ministry')
	:colour(0.113725,0.760784,0.266667)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		42)
f:govtype_weight('PLUTOCRATIC',		17)

f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('SLAVES',		49)
f:illegal_goods_probability('HAND_WEAPONS',		54)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('True Republic')


