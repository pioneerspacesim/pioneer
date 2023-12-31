-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Obsidian Systems')
	:description_short('Obsidian Systems')
	:description('Very little is currently known about The Obsidian Systems')
	:homeworld(-9,28,28,0,7)
	:foundingDate(3137)
	:expansionRate(1.50153)
	:military_name('Systems Militia')
	:police_name('Systems Security')
	:colour(0.607843,0.827451,0.239216)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		47)
f:govtype_weight('SOCDEM',		47)
f:govtype_weight('PLUTOCRATIC',		22)
f:govtype_weight('COMMUNIST',		22)

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('robots',		31)
f:illegal_goods_probability('slaves',		78)
f:illegal_goods_probability('nerve_gas',		71)

f:add_to_factions('Obsidian Systems')
