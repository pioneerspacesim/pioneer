-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim Council')
	:description_short('Rim Council')
	:description('Very little is currently known about The Rim Council')
	:homeworld(33,45,45,0,1)
	:foundingDate(3122)
	:expansionRate(1.32809)
	:military_name('Council Legion')
	:police_name('Rim Interior Ministry')
	:colour(1,0.968628,0.564706)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		36)
f:govtype_weight('PLUTOCRATIC',		12)

f:illegal_goods_probability('LIVE_ANIMALS',		80)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		100)

f:add_to_factions('Rim Council')


