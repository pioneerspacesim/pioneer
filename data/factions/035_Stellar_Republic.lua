-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Stellar Republic')
	:description_short('Stellar Republic')
	:description('Very little is currently known about The Stellar Republic')
	:homeworld(-27,38,38,0,13)
	:foundingDate(3087)
	:expansionRate(0.924386)
	:military_name('Republic Legion')
	:police_name('Stellar Prefecture')
	:colour(0.490196,0.905882,0.298039)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		28)
f:govtype_weight('COMMUNIST',		7)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		47)
f:illegal_goods_probability('nerve_gas',		49)

f:add_to_factions('Stellar Republic')
