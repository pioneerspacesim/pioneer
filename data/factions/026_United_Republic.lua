-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Republic')
	:description_short('United Republic')
	:description('Very little is currently known about The United Republic')
	:homeworld(-54,51,51,2,8)
	:foundingDate(3147)
	:expansionRate(1.619)
	:military_name('United Defense Force')
	:police_name('Republic Justiciars')
	:colour(0.792157,0,0.301961)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		71)
f:govtype_weight('COMMUNIST',		50)

f:illegal_goods_probability('LIVE_ANIMALS',		89)
f:illegal_goods_probability('SLAVES',		77)
f:illegal_goods_probability('BATTLE_WEAPONS',		56)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('United Republic')


