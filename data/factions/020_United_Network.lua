-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Network')
	:description_short('United Network')
	:description('Very little is currently known about The United Network')
	:homeworld(-43,-56,-56,1,11)
	:foundingDate(3158)
	:expansionRate(1.73861)
	:military_name('United War Fleet')
	:police_name('Network Security')
	:colour(1,1,0.6)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		35)

f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		57)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('United Network')


