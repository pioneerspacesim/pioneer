-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('robots',		62)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		65)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Independent Republic')
