-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Alliance')
	:description_short('United Alliance')
	:description('Very little is currently known about The United Alliance')
	:homeworld(47,51,51,1,19)
	:foundingDate(3129)
	:expansionRate(1.4132)
	:military_name('Alliance War Fleet')
	:police_name('United Interior Ministry')
	:colour(0.756863,0.239216,0.0392157)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		76)
f:govtype_weight('MILDICT2',		76)
f:govtype_weight('LIBDEM',		57)
f:govtype_weight('DISORDER',		57)

f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('narcotics',		78)

f:add_to_factions('United Alliance')
