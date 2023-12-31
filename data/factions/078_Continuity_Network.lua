-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Continuity Network')
	:description_short('Continuity Network')
	:description('Very little is currently known about The Continuity Network')
	:homeworld(-58,-25,-25,0,11)
	:foundingDate(3156)
	:expansionRate(1.72432)
	:military_name('Network War Fleet')
	:police_name('Continuity Police')
	:colour(0.172549,0.796079,0.572549)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		13)
f:govtype_weight('COMMUNIST',		13)
f:govtype_weight('CORPORATE',		1)
f:govtype_weight('MILDICT2',		1)

f:illegal_goods_probability('animal_meat',		84)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('liquor',		39)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('hand_weapons',		38)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		61)

f:add_to_factions('Continuity Network')
