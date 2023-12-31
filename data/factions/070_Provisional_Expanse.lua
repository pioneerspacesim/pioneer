-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Provisional Expanse')
	:description_short('Provisional Expanse')
	:description('Very little is currently known about The Provisional Expanse')
	:homeworld(7,49,48,1,8)
	:foundingDate(3122)
	:expansionRate(1.33348)
	:military_name('Expanse War Fleet')
	:police_name('Expanse Police')
	:colour(1,1,0.854902)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		24)
f:govtype_weight('COMMUNIST',		24)
f:govtype_weight('CORPORATE',		5)
f:govtype_weight('MILDICT2',		5)

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('liquor',		42)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		60)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		28)

f:add_to_factions('Provisional Expanse')
