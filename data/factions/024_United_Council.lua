-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Council')
	:description_short('United Council')
	:description('Very little is currently known about The United Council')
	:homeworld(-18,-6,-6,1,10)
	:foundingDate(3054)
	:expansionRate(0.551866)
	:military_name('United Navy')
	:police_name('Council Prefecture')
	:colour(0.819608,0.352941,0.176471)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		33)
f:govtype_weight('PLUTOCRATIC',		33)
f:govtype_weight('CORPORATE',		11)
f:govtype_weight('LIBDEM',		3)

f:illegal_goods_probability('LIVE_ANIMALS',		55)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		64)
f:illegal_goods_probability('BATTLE_WEAPONS',		82)
f:illegal_goods_probability('NERVE_GAS',		52)

f:add_to_factions('United Council')


