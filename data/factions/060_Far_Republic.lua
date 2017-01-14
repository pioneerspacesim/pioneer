-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Far Republic')
	:description_short('Far Republic')
	:description('Very little is currently known about The Far Republic')
	:homeworld(44,-34,-34,0,4)
	:foundingDate(3154)
	:expansionRate(1.69827)
	:military_name('Republic Defense Force')
	:police_name('Republic Police')
	:colour(0.960784,0.411765,0.403922)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		4)
f:govtype_weight('PLUTOCRATIC',		4)

f:illegal_goods_probability('LIVE_ANIMALS',		75)
f:illegal_goods_probability('ROBOTS',		70)
f:illegal_goods_probability('SLAVES',		35)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Far Republic')


