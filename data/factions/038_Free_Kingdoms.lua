-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Free Kingdoms')
	:description_short('Free Kingdoms')
	:description('Very little is currently known about The Free Kingdoms')
	:homeworld(-28,-58,-58,1,2)
	:foundingDate(3112)
	:expansionRate(1.21615)
	:military_name('Free War Fleet')
	:police_name('Kingdoms Prefecture')
	:colour(1,0.435294,0.67451)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		71)
f:govtype_weight('LIBDEM',		71)
f:govtype_weight('MILDICT1',		50)
f:govtype_weight('SOCDEM',		50)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		79)
f:illegal_goods_probability('battle_weapons',		100)

f:add_to_factions('Free Kingdoms')
