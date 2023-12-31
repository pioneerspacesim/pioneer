-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Dagger Council')
	:description_short('Dagger Council')
	:description('Very little is currently known about The Dagger Council')
	:homeworld(-25,-53,-53,0,1)
	:foundingDate(3066)
	:expansionRate(0.684718)
	:military_name('Council Navy')
	:police_name('Council Inquisition')
	:colour(0.662745,0.831373,0.678431)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		54)
f:govtype_weight('PLUTOCRATIC',		29)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('robots',		44)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		59)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('narcotics',		55)

f:add_to_factions('Dagger Council')
