-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Provisional Commonwealth')
	:description_short('Provisional Commonwealth')
	:description('Very little is currently known about The Provisional Commonwealth')
	:homeworld(-34,-27,-27,0,11)
	:foundingDate(3102)
	:expansionRate(1.10041)
	:military_name('Commonwealth Legion')
	:police_name('Commonwealth Constabulary')
	:colour(0.301961,0.380392,1)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		18)
f:govtype_weight('LIBDEM',		18)
f:govtype_weight('MILDICT1',		3)
f:govtype_weight('SOCDEM',		3)

f:illegal_goods_probability('ANIMAL_MEAT',		50)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('LIQUOR',		61)
f:illegal_goods_probability('SLAVES',		41)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		32)

f:add_to_factions('Provisional Commonwealth')


