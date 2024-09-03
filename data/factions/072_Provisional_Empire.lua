-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Provisional Empire')
	:description_short('Provisional Empire')
	:description('Very little is currently known about The Provisional Empire')
	:homeworld(-39,21,20,1,20)
	:foundingDate(3118)
	:expansionRate(1.28406)
	:military_name('Provisional Defense Force')
	:police_name('Empire Prefecture')
	:colour(0.623529,1,1)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		27)
f:govtype_weight('SOCDEM',		27)
f:govtype_weight('PLUTOCRATIC',		7)
f:govtype_weight('COMMUNIST',		7)
f:govtype_weight('MILDICT1',		1)
f:govtype_weight('MILDICT2',		1)

f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('narcotics',		60)

f:add_to_factions('Provisional Empire')
