-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Kingdoms')
	:description_short('United Kingdoms')
	:description('Very little is currently known about The United Kingdoms')
	:homeworld(39,31,31,0,9)
	:foundingDate(3161)
	:expansionRate(1.77144)
	:military_name('United Space Arm')
	:police_name('United Police')
	:colour(0.596078,0.411765,0.231373)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		6)
f:govtype_weight('PLUTOCRATIC',		0)

f:illegal_goods_probability('LIVE_ANIMALS',		35)
f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('NERVE_GAS',		65)
f:illegal_goods_probability('NARCOTICS',		60)

f:add_to_factions('United Kingdoms')


