-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Expanse')
	:description_short('True Expanse')
	:description('Very little is currently known about The True Expanse')
	:homeworld(-21,14,14,1,10)
	:foundingDate(3099)
	:expansionRate(1.06996)
	:military_name('Expanse Space Arm')
	:police_name('True Police')
	:colour(0.894118,0.0509804,1)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		1)
f:govtype_weight('CORPORATE',		1)
f:govtype_weight('DISORDER',		0)
f:govtype_weight('LIBDEM',		0)

f:illegal_goods_probability('slaves',		61)
f:illegal_goods_probability('nerve_gas',		41)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('True Expanse')
