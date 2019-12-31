-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Liberation Faction')
	:description_short('Liberation Faction')
	:description('Very little is currently known about The Liberation Faction')
	:homeworld(-32,-7,-7,0,8)
	:foundingDate(3099)
	:expansionRate(1.06621)
	:military_name('Faction Navy')
	:police_name('Liberation Inquisition')
	:colour(0.396078,1,0.380392)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		49)
f:govtype_weight('SOCDEM',		49)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		51)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		39)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Liberation Faction')


