-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Outer Republic')
	:description_short('Outer Republic')
	:description('Very little is currently known about The Outer Republic')
	:homeworld(-53,52,52,0,9)
	:foundingDate(3077)
	:expansionRate(0.820246)
	:military_name('Outer Battle Flight')
	:police_name('Outer Prefecture')
	:colour(1,0.372549,0.101961)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		20)
f:govtype_weight('PLUTOCRATIC',		20)
f:govtype_weight('CORPORATE',		4)
f:govtype_weight('LIBDEM',		0)

f:illegal_goods_probability('LIVE_ANIMALS',		63)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		49)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		88)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Outer Republic')


