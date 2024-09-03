-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Obsidian Alliance')
	:description_short('Obsidian Alliance')
	:description('Very little is currently known about The Obsidian Alliance')
	:homeworld(-28,28,28,1,19)
	:foundingDate(3110)
	:expansionRate(1.19068)
	:military_name('Alliance Legion')
	:police_name('Obsidian Police')
	:colour(1,0.313726,0.721569)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		47)
f:govtype_weight('DISORDER',		47)
f:govtype_weight('SOCDEM',		22)
f:govtype_weight('LIBDEM',		10)

f:illegal_goods_probability('animal_meat',		52)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('nerve_gas',		58)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Obsidian Alliance')
