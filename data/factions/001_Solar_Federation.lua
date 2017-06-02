-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Solar Federation')
	:description_short('The historical birthplace of humankind')
	:description('Sol is a fine joint')
	:homeworld(0,0,0,0,4)
	:foundingDate(3050)
	:expansionRate(1)
	:military_name('SolFed Military')
	:police_name('SolFed Police Force')
	:police_ship('kanara')
	:colour(0.4,0.4,1)

f:govtype_weight('EARTHDEMOC',		60)
f:govtype_weight('EARTHCOLONIAL',		40)

f:illegal_goods_probability('ANIMAL_MEAT',		75)
f:illegal_goods_probability('LIVE_ANIMALS',		75)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		50)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Solar Federation')


