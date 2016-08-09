-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Continuity Alliance')
	:description_short('Continuity Alliance')
	:description('Very little is currently known about The Continuity Alliance')
	:homeworld(59,40,40,0,12)
	:foundingDate(3135)
	:expansionRate(1.48467)
	:military_name('Continuity Navy')
	:police_name('Continuity Security')
	:colour(1,0.184314,0.14902)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		50)
f:govtype_weight('PLUTOCRATIC',		50)
f:govtype_weight('CORPORATE',		25)

f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('ROBOTS',		73)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		52)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)

f:add_to_factions('Continuity Alliance')


