-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Obsidian Worlds')
	:description_short('Obsidian Worlds')
	:description('Very little is currently known about The Obsidian Worlds')
	:homeworld(-60,-44,-44,0,17)
	:foundingDate(3052)
	:expansionRate(0.529014)
	:military_name('Worlds War Fleet')
	:police_name('Worlds Security')
	:colour(1,0.117647,0.301961)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		7)
f:govtype_weight('DISORDER',		7)

f:illegal_goods_probability('hand_weapons',		87)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Obsidian Worlds')
