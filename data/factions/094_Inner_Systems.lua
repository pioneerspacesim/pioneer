-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Inner Systems')
	:description_short('Inner Systems')
	:description('Very little is currently known about The Inner Systems')
	:homeworld(4,51,51,0,14)
	:foundingDate(3143)
	:expansionRate(1.57082)
	:military_name('Inner Space Arm')
	:police_name('Inner Police')
	:colour(0.0117647,0.411765,1)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		4)
f:govtype_weight('COMMUNIST',		4)
f:govtype_weight('CORPORATE',		0)
f:govtype_weight('MILDICT2',		0)

f:illegal_goods_probability('animal_meat',		43)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Inner Systems')
