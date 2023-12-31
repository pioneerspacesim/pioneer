-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood League')
	:description_short('Blood League')
	:description('Very little is currently known about The Blood League')
	:homeworld(46,42,42,1,9)
	:foundingDate(3174)
	:expansionRate(1.92995)
	:military_name('Blood Defense Wing')
	:police_name('League Interior Ministry')
	:colour(0.929412,0.945098,0.482353)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		78)
f:govtype_weight('COMMUNIST',		78)
f:govtype_weight('CORPORATE',		60)
f:govtype_weight('MILDICT2',		60)

f:illegal_goods_probability('animal_meat',		32)
f:illegal_goods_probability('live_animals',		61)
f:illegal_goods_probability('liquor',		56)
f:illegal_goods_probability('slaves',		26)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Blood League')
