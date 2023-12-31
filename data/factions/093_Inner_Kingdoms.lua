-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Inner Kingdoms')
	:description_short('Inner Kingdoms')
	:description('Very little is currently known about The Inner Kingdoms')
	:homeworld(-33,-55,-54,2,3)
	:foundingDate(3118)
	:expansionRate(1.27892)
	:military_name('Inner Space Arm')
	:police_name('Inner Interior Ministry')
	:colour(0.113725,0.0509804,1)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		6)
f:govtype_weight('SOCDEM',		6)
f:govtype_weight('PLUTOCRATIC',		0)
f:govtype_weight('COMMUNIST',		0)

f:illegal_goods_probability('slaves',		53)
f:illegal_goods_probability('hand_weapons',		96)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		93)

f:add_to_factions('Inner Kingdoms')
