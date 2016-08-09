-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Outer League')
	:description_short('Outer League')
	:description('Very little is currently known about The Outer League')
	:homeworld(-6,-30,-30,0,4)
	:foundingDate(3161)
	:expansionRate(1.77446)
	:military_name('Outer War Fleet')
	:police_name('League Interior Ministry')
	:colour(1,1,1)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		77)

f:illegal_goods_probability('SLAVES',		78)
f:illegal_goods_probability('BATTLE_WEAPONS',		30)
f:illegal_goods_probability('NERVE_GAS',		100)

f:add_to_factions('Outer League')


