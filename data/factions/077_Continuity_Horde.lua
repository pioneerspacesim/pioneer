-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Continuity Horde')
	:description_short('Continuity Horde')
	:description('Very little is currently known about The Continuity Horde')
	:homeworld(49,54,54,0,5)
	:foundingDate(3116)
	:expansionRate(1.25993)
	:military_name('Horde Militia')
	:police_name('Continuity Prefecture')
	:colour(0.85098,0.87451,0.152941)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		29)
f:govtype_weight('LIBDEM',		29)
f:govtype_weight('MILDICT1',		8)
f:govtype_weight('SOCDEM',		8)
f:govtype_weight('DISORDER',		2)
f:govtype_weight('COMMUNIST',		2)

f:illegal_goods_probability('live_animals',		36)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		87)
f:illegal_goods_probability('narcotics',		95)

f:add_to_factions('Continuity Horde')
