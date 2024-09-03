-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Solar Federation')
	:description_short('The historical birthplace of humankind')
	:description('Sol is a fine joint')
	:homeworld(0,0,0,0,16) --Mars
	:foundingDate(3050)
	:expansionRate(1)
	:military_name('SolFed Military')
	:police_name('SolFed Police Force')
	:police_ship('kanara')
	:colour(0.4,0.4,1)

f:govtype_weight('EARTHDEMOC',		60)
f:govtype_weight('EARTHCOLONIAL',		40)

f:illegal_goods_probability('animal_meat',		75)
f:illegal_goods_probability('live_animals',		75)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		50)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Solar Federation')
