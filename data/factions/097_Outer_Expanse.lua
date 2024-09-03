-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Outer Expanse')
	:description_short('Outer Expanse')
	:description('Very little is currently known about The Outer Expanse')
	:homeworld(-13,-13,-12,0,11)
	:foundingDate(3135)
	:expansionRate(1.48027)
	:military_name('Expanse War Fleet')
	:police_name('Outer Inquisition')
	:colour(1,1,1)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		23)
f:govtype_weight('COMMUNIST',		5)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		37)

f:add_to_factions('Outer Expanse')
