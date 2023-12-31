-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood Empire')
	:description_short('Blood Empire')
	:description('Very little is currently known about The Blood Empire')
	:homeworld(-40,18,18,0,12)
	:foundingDate(3111)
	:expansionRate(1.2033)
	:military_name('Blood Defense Force')
	:police_name('Blood Constabulary')
	:colour(0.439216,1,0.686275)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		68)
f:govtype_weight('CORPORATE',		68)
f:govtype_weight('DISORDER',		46)
f:govtype_weight('LIBDEM',		46)

f:illegal_goods_probability('battle_weapons',		57)
f:illegal_goods_probability('nerve_gas',		73)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Blood Empire')
