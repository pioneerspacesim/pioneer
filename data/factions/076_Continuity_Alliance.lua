-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Continuity Alliance')
	:description_short('Continuity Alliance')
	:description('Very little is currently known about The Continuity Alliance')
	:homeworld(59,40,40,0,12)
	:foundingDate(3135)
	:expansionRate(1.48467)
	:military_name('Continuity Navy')
	:police_name('Continuity Security')
	:colour(1,0.184314,0.14902)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		50)
f:govtype_weight('PLUTOCRATIC',		50)
f:govtype_weight('CORPORATE',		25)

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('robots',		73)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		52)
f:illegal_goods_probability('battle_weapons',		100)

f:add_to_factions('Continuity Alliance')
