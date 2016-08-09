-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Obsidian Commonwealth')
	:description_short('Obsidian Commonwealth')
	:description('Very little is currently known about The Obsidian Commonwealth')
	:homeworld(50,-53,-54,0,20)
	:foundingDate(3151)
	:expansionRate(1.65699)
	:military_name('Commonwealth Militia')
	:police_name('Obsidian Interior Ministry')
	:colour(0.592157,0.976471,0.239216)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		33)
f:govtype_weight('LIBDEM',		33)
f:govtype_weight('MILDICT1',		11)
f:govtype_weight('SOCDEM',		11)
f:govtype_weight('DISORDER',		3)
f:govtype_weight('COMMUNIST',		3)

f:illegal_goods_probability('LIVE_ANIMALS',		29)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		55)
f:illegal_goods_probability('NERVE_GAS',		75)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Obsidian Commonwealth')


