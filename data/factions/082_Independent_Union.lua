-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent Union')
	:description_short('Independent Union')
	:description('Very little is currently known about The Independent Union')
	:homeworld(44,40,39,1,7)
	:foundingDate(3152)
	:expansionRate(1.67312)
	:military_name('Independent War Fleet')
	:police_name('Independent Inquisition')
	:colour(0.345098,0.8,0.227451)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		64)
f:govtype_weight('DISORDER',		64)
f:govtype_weight('SOCDEM',		41)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('liquor',		43)
f:illegal_goods_probability('slaves',		74)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Independent Union')
