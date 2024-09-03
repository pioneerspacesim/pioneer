-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim Worlds')
	:description_short('Rim Worlds')
	:description('Very little is currently known about The Rim Worlds')
	:homeworld(47,-55,-55,0,14)
	:foundingDate(3180)
	:expansionRate(1.99778)
	:military_name('Rim Battle Flight')
	:police_name('Worlds Interior Ministry')
	:colour(1,1,0.878431)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		35)
f:govtype_weight('MILDICT2',		35)
f:govtype_weight('LIBDEM',		12)
f:govtype_weight('DISORDER',		12)

f:illegal_goods_probability('animal_meat',		66)
f:illegal_goods_probability('liquor',		80)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		84)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('narcotics',		70)

f:add_to_factions('Rim Worlds')
