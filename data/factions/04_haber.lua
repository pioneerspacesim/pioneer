-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Haber Corporation')
	:description_short('Haber Corporation')
	:description('Totalitarian corporate state controlled by a board of CEOs')
	:homeworld(4,-9,-17,0,21)
	:foundingDate(3150.0)
	:expansionRate(0.5)
	:military_name('Haber Fleet Division')
	:police_name('Haber Enforcement Division')
	:colour(1.0,0.4,0.4)

f:govtype_weight('CORPORATE',    100)

f:illegal_goods_probability('HAND_WEAPONS',100)	-- empire/etc
f:illegal_goods_probability('BATTLE_WEAPONS',100)	--empire/etc
f:illegal_goods_probability('NERVE_GAS',100)--empire
f:illegal_goods_probability('SLAVES',100)--empire

f:add_to_factions('Haber')
