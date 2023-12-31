-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True League')
	:description_short('True League')
	:description('Very little is currently known about The True League')
	:homeworld(-27,-59,-59,2,8)
	:foundingDate(3121)
	:expansionRate(1.31987)
	:military_name('True Space Arm')
	:police_name('League Security')
	:colour(0.294118,0.517647,0.152941)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		71)
f:govtype_weight('LIBDEM',		71)
f:govtype_weight('MILDICT1',		50)
f:govtype_weight('SOCDEM',		50)

f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		88)
f:illegal_goods_probability('narcotics',		35)

f:add_to_factions('True League')
