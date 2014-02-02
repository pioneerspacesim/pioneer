-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Red')
	:description_short('Not Blue')
	:description('The RED team are not Blue')
	:homeworld(4,-9,-17,0,21)
	:foundingDate(3150.0)
	:expansionRate(3.0)
	:military_name('Red Fleet')
	:police_name('Red Right Hands')
	:colour(1.0,0.4,0.4)

f:govtype_weight('EMPIRERULE',    80)
f:govtype_weight('EMPIREMILDICT', 20)

f:illegal_goods_probability('LIQUOR',88)	-- independent/empire
f:illegal_goods_probability('HAND_WEAPONS',50)	-- empire/etc
f:illegal_goods_probability('BATTLE_WEAPONS',100)	--empire/etc
f:illegal_goods_probability('NERVE_GAS',90)--empire
f:illegal_goods_probability('NARCOTICS',50)--empire
f:illegal_goods_probability('SLAVES',94)--empire

f:add_to_factions('Red')
