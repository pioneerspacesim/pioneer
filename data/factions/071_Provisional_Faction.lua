-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Provisional Faction')
	:description_short('Provisional Faction')
	:description('Very little is currently known about The Provisional Faction')
	:homeworld(-8,-31,-31,0,5)
	:foundingDate(3103)
	:expansionRate(1.10971)
	:military_name('Provisional Navy')
	:police_name('Provisional Justiciars')
	:colour(0.45098,0.682353,0.305882)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		30)
f:govtype_weight('CORPORATE',		30)
f:govtype_weight('DISORDER',		9)
f:govtype_weight('LIBDEM',		9)
f:govtype_weight('SOCDEM',		2)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		66)
f:illegal_goods_probability('NARCOTICS',		72)

f:add_to_factions('Provisional Faction')


