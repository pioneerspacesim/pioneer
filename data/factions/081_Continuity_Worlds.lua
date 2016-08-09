-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Continuity Worlds')
	:description_short('Continuity Worlds')
	:description('Very little is currently known about The Continuity Worlds')
	:homeworld(-17,19,19,1,10)
	:foundingDate(3114)
	:expansionRate(1.23823)
	:military_name('Continuity Legion')
	:police_name('Continuity Constabulary')
	:colour(0.54902,0.47451,0.462745)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		79)
f:govtype_weight('SOCDEM',		79)
f:govtype_weight('PLUTOCRATIC',		62)
f:govtype_weight('COMMUNIST',		62)

f:illegal_goods_probability('LIVE_ANIMALS',		57)
f:illegal_goods_probability('ROBOTS',		46)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('BATTLE_WEAPONS',		51)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Continuity Worlds')


