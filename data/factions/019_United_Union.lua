-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Union')
	:description_short('United Union')
	:description('Very little is currently known about The United Union')
	:homeworld(48,-17,-17,1,7)
	:foundingDate(3101)
	:expansionRate(1.08399)
	:military_name('United Battle Flight')
	:police_name('Union Police')
	:colour(0.968628,0.690196,0.498039)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		66)
f:govtype_weight('MILDICT2',		66)

f:illegal_goods_probability('ANIMAL_MEAT',		100)
f:illegal_goods_probability('LIVE_ANIMALS',		33)
f:illegal_goods_probability('LIQUOR',		87)
f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		70)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('United Union')


