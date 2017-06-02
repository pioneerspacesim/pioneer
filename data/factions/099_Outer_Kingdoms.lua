-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Outer Kingdoms')
	:description_short('Outer Kingdoms')
	:description('Very little is currently known about The Outer Kingdoms')
	:homeworld(-15,-8,-8,0,8)
	:foundingDate(3114)
	:expansionRate(1.23968)
	:military_name('Outer War Fleet')
	:police_name('Outer Police')
	:colour(0.717647,0.337255,0.772549)

f:govtype_weight('MILDICT2',		100)
f:govtype_weight('COMMUNIST',		53)
f:govtype_weight('DISORDER',		53)
f:govtype_weight('SOCDEM',		28)

f:illegal_goods_probability('HAND_WEAPONS',		89)
f:illegal_goods_probability('BATTLE_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		56)

f:add_to_factions('Outer Kingdoms')


