-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood Faction')
	:description_short('Blood Faction')
	:description('Very little is currently known about The Blood Faction')
	:homeworld(26,9,9,1,15)
	:foundingDate(3151)
	:expansionRate(1.66219)
	:military_name('Faction Defense Force')
	:police_name('Faction Constabulary')
	:colour(1,0.52549,0.137255)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		18)
f:govtype_weight('MILDICT2',		18)
f:govtype_weight('LIBDEM',		3)
f:govtype_weight('DISORDER',		3)
f:govtype_weight('CORPORATE',		0)

f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		78)

f:add_to_factions('Blood Faction')
