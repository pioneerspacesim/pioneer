-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent Network')
	:description_short('Independent Network')
	:description('Very little is currently known about The Independent Network')
	:homeworld(-43,-23,-23,0,4)
	:foundingDate(3137)
	:expansionRate(1.49871)
	:military_name('Network War Fleet')
	:police_name('Independent Constabulary')
	:colour(0.835294,0.305882,0.2)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		12)
f:govtype_weight('CORPORATE',		12)

f:illegal_goods_probability('liquor',		59)
f:illegal_goods_probability('slaves',		51)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('nerve_gas',		80)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Independent Network')
