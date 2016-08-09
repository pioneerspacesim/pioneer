-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim Territories')
	:description_short('Rim Territories')
	:description('Very little is currently known about The Rim Territories')
	:homeworld(37,12,12,1,9)
	:foundingDate(3127)
	:expansionRate(1.39059)
	:military_name('Rim Defense Wing')
	:police_name('Territories Security')
	:colour(0.0196078,0.784314,0.298039)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		25)
f:govtype_weight('COMMUNIST',		25)

f:illegal_goods_probability('LIVE_ANIMALS',		40)
f:illegal_goods_probability('ROBOTS',		100)
f:illegal_goods_probability('SLAVES',		98)
f:illegal_goods_probability('HAND_WEAPONS',		59)
f:illegal_goods_probability('BATTLE_WEAPONS',		62)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Rim Territories')


