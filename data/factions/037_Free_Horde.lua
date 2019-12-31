-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Free Horde')
	:description_short('Free Horde')
	:description('Very little is currently known about The Free Horde')
	:homeworld(-19,-42,-42,0,6)
	:foundingDate(3143)
	:expansionRate(1.56781)
	:military_name('Free Militia')
	:police_name('Free Constabulary')
	:colour(0.984314,0.701961,1)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		38)
f:govtype_weight('COMMUNIST',		14)

f:illegal_goods_probability('LIVE_ANIMALS',		65)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)

f:add_to_factions('Free Horde')


