-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Far Faction')
	:description_short('Far Faction')
	:description('Very little is currently known about The Far Faction')
	:homeworld(40,-54,-54,0,5)
	:foundingDate(3051)
	:expansionRate(0.514857)
	:military_name('Far Space Arm')
	:police_name('Far Police')
	:colour(0.372549,0.72549,0.109804)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		48)
f:govtype_weight('DISORDER',		48)
f:govtype_weight('SOCDEM',		23)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('liquor',		67)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		42)
f:illegal_goods_probability('nerve_gas',		99)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Far Faction')
