-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Obsidian Union')
	:description_short('Obsidian Union')
	:description('Very little is currently known about The Obsidian Union')
	:homeworld(-42,57,57,1,2)
	:foundingDate(3165)
	:expansionRate(1.82033)
	:military_name('Union Militia')
	:police_name('Union Inquisition')
	:colour(0.945098,0.956863,0.745098)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		27)
f:govtype_weight('PLUTOCRATIC',		7)
f:govtype_weight('CORPORATE',		1)

f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('LIQUOR',		75)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		40)
f:illegal_goods_probability('BATTLE_WEAPONS',		47)
f:illegal_goods_probability('NERVE_GAS',		84)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Obsidian Union')


