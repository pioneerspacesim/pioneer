-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Obsidian Horde')
	:description_short('Obsidian Horde')
	:description('Very little is currently known about The Obsidian Horde')
	:homeworld(15,3,3,0,7)
	:foundingDate(3171)
	:expansionRate(1.89544)
	:military_name('Obsidian Militia')
	:police_name('Horde Prefecture')
	:colour(0.494118,0.972549,0.0156863)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		1)
f:govtype_weight('CORPORATE',		1)

f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('narcotics',		80)

f:add_to_factions('Obsidian Horde')
