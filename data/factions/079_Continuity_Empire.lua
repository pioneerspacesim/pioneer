-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Continuity Empire')
	:description_short('Continuity Empire')
	:description('Very little is currently known about The Continuity Empire')
	:homeworld(-54,-33,-32,3,13)
	:foundingDate(3092)
	:expansionRate(0.982215)
	:military_name('Empire Defense Force')
	:police_name('Continuity Security')
	:colour(0.0588235,1,0.176471)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		48)
f:govtype_weight('PLUTOCRATIC',		23)

f:illegal_goods_probability('live_animals',		47)
f:illegal_goods_probability('hand_weapons',		62)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Continuity Empire')
