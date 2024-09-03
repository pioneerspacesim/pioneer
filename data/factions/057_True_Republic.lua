-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('slaves',		49)
f:illegal_goods_probability('hand_weapons',		54)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('True Republic')
