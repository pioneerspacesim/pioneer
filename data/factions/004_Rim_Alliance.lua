-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim Alliance')
	:description_short('Rim Alliance')
	:description('Very little is currently known about The Rim Alliance')
	:homeworld(56,35,35,1,16)
	:foundingDate(3138)
	:expansionRate(1.50992)
	:military_name('Rim Space Arm')
	:police_name('Alliance Security')
	:colour(0.564706,0.431373,1)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		28)

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('liquor',		70)
f:illegal_goods_probability('slaves',		98)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		70)

f:add_to_factions('Rim Alliance')
