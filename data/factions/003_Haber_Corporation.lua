-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Haber Corporation')
	:description_short('Haber Corporation')
	:description('Totalitarian corporate state controlled by a board of CEOs')
	:homeworld(4,-9,-17,0,21)
	:foundingDate(3150)
	:expansionRate(0.5)
	:military_name('Haber Fleet Division')
	:police_name('Haber Enforcement Division')
	:colour(1,0.4,0.4)

f:govtype_weight('CORPORATE',		100)

f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Haber Corporation')
