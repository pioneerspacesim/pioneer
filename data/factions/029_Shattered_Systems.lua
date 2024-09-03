-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Shattered Systems')
	:description_short('Shattered Systems')
	:description('Very little is currently known about The Shattered Systems')
	:homeworld(44,-7,-7,0,7)
	:foundingDate(3174)
	:expansionRate(1.92202)
	:military_name('Shattered Guards')
	:police_name('Shattered Security')
	:colour(0.647059,0.388235,0.262745)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		54)
f:govtype_weight('SOCDEM',		54)
f:govtype_weight('PLUTOCRATIC',		29)
f:govtype_weight('COMMUNIST',		29)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('nerve_gas',		99)

f:add_to_factions('Shattered Systems')
