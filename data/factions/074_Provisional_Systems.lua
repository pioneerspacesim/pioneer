-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Provisional Systems')
	:description_short('Provisional Systems')
	:description('Very little is currently known about The Provisional Systems')
	:homeworld(25,38,37,1,2)
	:foundingDate(3174)
	:expansionRate(1.92084)
	:military_name('Provisional Space Arm')
	:police_name('Provisional Prefecture')
	:colour(0.764706,1,0.490196)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		31)
f:govtype_weight('DISORDER',		31)
f:govtype_weight('SOCDEM',		9)
f:govtype_weight('LIBDEM',		2)

f:illegal_goods_probability('live_animals',		94)
f:illegal_goods_probability('slaves',		97)
f:illegal_goods_probability('nerve_gas',		93)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Provisional Systems')
