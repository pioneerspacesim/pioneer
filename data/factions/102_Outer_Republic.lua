-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('live_animals',		63)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('slaves',		49)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		88)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Outer Republic')
